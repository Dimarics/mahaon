#ifndef ROSSCANSUBSCRIBER_H
#define ROSSCANSUBSCRIBER_H

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include <cmath> // для проверки на бесконечность

class RosScanSubscriber : public rclcpp::Node
{
public:
    RosScanSubscriber()
    : Node("lidar_subscriber_node") // Имя вашего узла
    {
        // Создаём подписчик на топик "/scan" с очередью на 10 сообщений
        subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan", 10,
            std::bind(&LidarSubscriber::topic_callback, this, std::placeholders::_1)
        );
        RCLCPP_INFO(this->get_logger(), "Lidar Subscriber Node has been started.");
    }

private:
    void topic_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) const
    {
        // msg->ranges содержит массив расстояний.
        // Всегда проверяйте, что массив не пуст, чтобы избежать ошибок.
        if (msg->ranges.empty()) {
            RCLCPP_WARN(this->get_logger(), "Received an empty scan!");
            return;
        }

        // Находим индекс, соответствующий центральному лучу (прямо перед роботом)
        size_t center_index = msg->ranges.size() / 2;
        float front_distance = msg->ranges[center_index];

        // Валидные данные не должны быть бесконечностью (inf) или "не числом" (nan)
        if (std::isinf(front_distance) || std::isnan(front_distance)) {
            RCLCPP_INFO(this->get_logger(), "Front: No obstacle detected (inf or nan)");
        } else {
            RCLCPP_INFO(this->get_logger(), "Front distance: %f meters", front_distance);
        }

        // Пример: как получить угол для этого луча [citation:3].
        // double angle = msg->angle_min + msg->angle_increment * center_index;
        // RCLCPP_INFO(this->get_logger(), "Angle of front ray: %f radians", angle);
    }

    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
};

#endif // ROSSCANSUBSCRIBER_H
