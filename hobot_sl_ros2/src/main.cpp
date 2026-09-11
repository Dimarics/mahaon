// #include <QCoreApplication>
#include "hobot_sl_ros2/rosbridge.h"
#include <rclcpp/utilities.hpp>

int main(int argc, char *argv[]) {
    // QCoreApplication a(argc, argv);
    // return a.exec();
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ROSbridge>());
    rclcpp::shutdown();
    return 0;
}
