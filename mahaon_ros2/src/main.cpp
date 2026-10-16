#include <rclcpp/utilities.hpp>
#include <boost/asio/io_context.hpp>
#include "mahaon/controller.h"

int main(int argc, char *argv[]) {

    rclcpp::init(argc, argv);
    boost::asio::io_context io_context;
    Controller controller(io_context);
    rclcpp::on_shutdown([&controller] {
        controller.shutdown();
    });
    io_context.run();
    rclcpp::shutdown();
    return EXIT_SUCCESS;
}
