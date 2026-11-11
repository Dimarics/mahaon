#include "mahaon/lidar_driver.h"
#include "mahaon/app_config.h"
#include "rclcpp/time.hpp"
#include <boost/asio/post.hpp>
#include <iostream>
#include <memory>

LidarDriver::LidarDriver(boost::asio::io_context &io_context)
    : m_running(true)
    , m_newScanCallback([](const std::shared_ptr<LaserScan>){})
    , m_newResampledScanCallback([](const sensor_msgs::msg::LaserScan::SharedPtr){})
{
    //ydlidar::os_init();
    m_thread = std::thread([&io_context, this]() {
        YAML::Node config = app_config::params["lidar_driver"];
        if (!config) {
            std::cout << "[lidar_driver] конфигурация лидара не найдена" << std::endl;
            return;
        }
        CYdLidar lidar;

        const std::string port         = config["port"].as<std::string>("");

        const int baudrate             = config["baudrate"].as<int>(115200);
        const int lidar_type           = config["lidar_type"].as<int>(TYPE_TRIANGLE);
        const int device_type          = config["device_type"].as<int>(YDLIDAR_TYPE_SERIAL);
        const int sample_rate          = config["sample_rate"].as<int>(3);
        const int abnormal_check_count = config["abnormal_check_count"].as<int>(4);

        const bool resolution_fixed    = config["resolution_fixed"].as<bool>(true);
        const bool reversion           = config["reversion"].as<bool>(false);
        const bool inverted            = config["inverted"].as<bool>(true);
        const bool auto_reconnect      = config["auto_reconnect"].as<bool>(true);
        const bool single_channel      = config["single_channel"].as<bool>(true);
        const bool intensity           = config["intensity"].as<bool>(false);
        const bool support_motor_dtr   = config["support_motor_dtr"].as<bool>(false);
        // const int invalid_range_is_inf = config["invalid_range_is_inf"].as<bool>(false);

        const float angle_max_deg      = config["angle_max"].as<float>(180.f);
        const float angle_min_deg      = config["angle_min"].as<float>(-180.f);
        const float range_max          = config["range_max"].as<float>(8.f);
        const float range_min          = config["range_min"].as<float>(0.12f);
        const float frequency          = config["frequency"].as<float>(10.f);

        const float angle_max          = angle_max_deg / 180.f * M_PI;
        const float angle_min          = angle_min_deg / 180.f * M_PI;

        lidar.setlidaropt(LidarPropSerialPort, port.c_str(), port.size());

        // массив игнорирования
        // std::string ignore_array;
        // ignore_array.clear();
        // lidar.setlidaropt(LidarPropIgnoreArray,         ignore_array.c_str(), ignore_array.size());

        lidar.setlidaropt(LidarPropSerialBaudrate,      &baudrate, sizeof(int));
        lidar.setlidaropt(LidarPropLidarType,           &lidar_type, sizeof(int));
        lidar.setlidaropt(LidarPropDeviceType,          &device_type, sizeof(int));
        lidar.setlidaropt(LidarPropSampleRate,          &sample_rate, sizeof(int));
        lidar.setlidaropt(LidarPropAbnormalCheckCount,  &abnormal_check_count, sizeof(int));

        lidar.setlidaropt(LidarPropFixedResolution,     &resolution_fixed, sizeof(bool));
        lidar.setlidaropt(LidarPropReversion,           &reversion, sizeof(bool));
        lidar.setlidaropt(LidarPropInverted,            &inverted, sizeof(bool));
        lidar.setlidaropt(LidarPropAutoReconnect,       &auto_reconnect, sizeof(bool));
        lidar.setlidaropt(LidarPropSingleChannel,       &single_channel, sizeof(bool));
        lidar.setlidaropt(LidarPropIntenstiy,           &intensity, sizeof(bool));
        lidar.setlidaropt(LidarPropSupportMotorDtrCtrl, &support_motor_dtr, sizeof(bool));

        lidar.setlidaropt(LidarPropMinAngle,            &angle_min_deg, sizeof(float));
        lidar.setlidaropt(LidarPropMaxAngle,            &angle_max_deg, sizeof(float));
        lidar.setlidaropt(LidarPropMinRange,            &range_min, sizeof(float));
        lidar.setlidaropt(LidarPropMaxRange,            &range_max, sizeof(float));
        lidar.setlidaropt(LidarPropScanFrequency,       &frequency, sizeof(float));

        bool ret = false;
        while (m_running && !ret) {
            ret = lidar.initialize();
            if (ret) {
                lidar.turnOn();
            } else {
                std::cout << "[lidar_driver] ошибка инициализации: " << lidar.DescribeError() << std::endl;
                std::cout << "[lidar_driver] reconnect..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        int mean_size = 0;
        {
            /*
            std::vector<int> scan_sizes(50, 0);
            for (int i = 0; m_running && ydlidar::os_isOk() && i < scan_sizes.size(); ++i) {
                LaserScan scan;
                if (!lidar.doProcessSimple(scan)) {
                    std::cout << "[lidar_driver] не удалось получить данные лидара" << std::endl;
                    continue;
                }
                scan_sizes[i] = scan.size;
            }
            */

            // вычисление среднего количества точек в скане
            const int mean_count = 50;
            int sum_size = 0;
            for (int i = 0; m_running && ydlidar::os_isOk() && i < mean_count; ++i) {
                LaserScan scan;
                if (!lidar.doProcessSimple(scan)) {
                    std::cout << "[lidar_driver] не удалось получить данные лидара" << std::endl;
                    continue;
                }
                sum_size += scan.size;
            }
            mean_size = sum_size / mean_count;
        }
        std::cout << "[lidar_driver] среднее количество точек в скане: " << mean_size << std::endl;

        const float angle_increment = (angle_max - angle_min) / (mean_size - 1);
        while (m_running && ydlidar::os_isOk()) {
            auto scan = std::make_shared<LaserScan>();
            if (!lidar.doProcessSimple(*scan)) {
                std::cout << "[lidar_driver] не удалось получить данные лидара" << std::endl;
                continue;
            }

            std::sort(scan->points.begin(), scan->points.end(), [](const LaserPoint &a, const LaserPoint &b) {
                return a.angle < b.angle;
            });
            boost::asio::post(io_context, [scan, this] {
                m_newScanCallback(scan);
            });

            auto resampled_scan = std::make_shared<sensor_msgs::msg::LaserScan>();
            resampled_scan->header.stamp = rclcpp::Time(scan->stamp);
            resampled_scan->header.frame_id = "laser";
            resampled_scan->angle_max = angle_max;
            resampled_scan->angle_min = angle_min;
            resampled_scan->range_max = range_max;
            resampled_scan->range_min = range_min;
            resampled_scan->angle_increment = angle_increment;
            resampled_scan->ranges.resize(mean_size);
            for (int i = 0; i < mean_size; ++i) {
                const float angle = angle_min + i * angle_increment;
                const float min_lim = angle - angle_increment / 2.f;
                const float max_lim = angle + angle_increment / 2.f;
                resampled_scan->ranges[i] = 0;
                for (auto &scan_point : scan->points) {
                    if (min_lim <= scan_point.angle && scan_point.angle < max_lim)
                        resampled_scan->ranges[i] = scan_point.range;
                }
            }
            boost::asio::post(io_context, [resampled_scan = std::move(resampled_scan), this] {
                m_newResampledScanCallback(resampled_scan);
            });
        }
        lidar.turnOff();
        lidar.disconnecting();
    });
}

void LidarDriver::shutdown() {
    m_running = false;
    m_thread.join();
}
