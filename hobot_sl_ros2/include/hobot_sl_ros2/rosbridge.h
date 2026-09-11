#ifndef ROSBRIDGE_H
#define ROSBRIDGE_H

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>

class ROSbridge : public rclcpp::Node
{
public:
    ROSbridge();

private:
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr m_subOdom;
    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg);
};

#endif // ROSBRIDGE_H
