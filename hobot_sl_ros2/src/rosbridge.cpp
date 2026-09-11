#include "hobot_sl_ros2/rosbridge.h"

ROSbridge::ROSbridge() : Node("hobot_slasher") {
    RCLCPP_INFO(get_logger(), "subscribe init");
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).reliable().transient_local();
    m_subOdom = create_subscription<nav_msgs::msg::Odometry>(
                "odom", qos,
                std::bind(&ROSbridge::odom_callback, this, std::placeholders::_1));
    RCLCPP_INFO(get_logger(), "subscribe success");
}

void ROSbridge::odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg) {
    // --- Позиция ---
    double x = msg->pose.pose.position.x;
    double y = msg->pose.pose.position.y;
    double z = msg->pose.pose.position.z;

    // --- Ориентация (кватернион) ---
    // double qx = msg->pose.pose.orientation.x;
    // double qy = msg->pose.pose.orientation.y;
    // double qz = msg->pose.pose.orientation.z;
    // double qw = msg->pose.pose.orientation.w;

    // --- Скорости ---
    double vx  = msg->twist.twist.linear.x;
    double vy  = msg->twist.twist.linear.y;
    double wz  = msg->twist.twist.angular.z;

    RCLCPP_INFO(get_logger(),
                "Поза: (%.2f, %.2f, %.2f) | Скорость: vx=%.2f, vy=%.2f, wz=%.2f",
                x, y, z, vx, vy, wz);
}
