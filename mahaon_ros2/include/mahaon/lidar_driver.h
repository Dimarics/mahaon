#ifndef LIDARDRIVER_H
#define LIDARDRIVER_H

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <src/CYdLidar.h>
#include <thread>

#if defined(_MSC_VER)
#pragma comment(lib, "ydlidar_sdk.lib")
#endif

class LidarDriver
{
public:
    LidarDriver(boost::asio::io_context &io_context);
    void shutdown();
    inline void onNewScanCallback(const std::function<void (const std::shared_ptr<LaserScan>)> &callback) {
        m_newScanCallback = callback;
    }
    inline void onNewResampledScanCallback(const std::function<void (const sensor_msgs::msg::LaserScan::SharedPtr)> &callback) {
        m_newResampledScanCallback = callback;
    }

private:
    std::thread m_thread;
    std::atomic_bool m_running;
    std::function<void (const std::shared_ptr<LaserScan>)> m_newScanCallback;
    std::function<void (const sensor_msgs::msg::LaserScan::SharedPtr)> m_newResampledScanCallback;
};

#endif // LIDARDRIVER_H
