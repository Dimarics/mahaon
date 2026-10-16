#include "mahaon/rosbridge.h"
#include <boost/asio/post.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <nav_msgs/msg/path.hpp>

ROSbridge::ROSbridge(boost::asio::io_context &io_context)
    : m_rosNode(std::make_shared<rclcpp::Node>("hobot_slasher"))
    , m_navigateActionClient(rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(m_rosNode, "navigate_to_pose"))
    , m_planActionClient(rclcpp_action::create_client<nav2_msgs::action::ComputePathToPose>(m_rosNode, "compute_path_to_pose"))
    , m_ioContext(io_context)
    , m_tfBuffer(m_rosNode->get_clock())
    , m_tfListener(m_tfBuffer)
    , m_tfTimer(io_context, std::chrono::milliseconds(100))
{
    m_rosThread = std::thread([this] {
        auto sub_scan = m_rosNode->create_subscription<sensor_msgs::msg::LaserScan>
                ("/scan", rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile(),
                 [this](const sensor_msgs::msg::LaserScan::SharedPtr laser_scan) {
            boost::asio::post(m_ioContext, [laser_scan = std::move(laser_scan), this] {
                m_scanReceivedCallback(laser_scan);
            });
        });
        auto sub_odom = m_rosNode->create_subscription<nav_msgs::msg::Odometry>
                ("/odom", rclcpp::QoS(rclcpp::KeepLast(10)).reliable().transient_local(),
                 [this](const nav_msgs::msg::Odometry::SharedPtr odom) {
            double x = odom->pose.pose.position.x;
            double y = odom->pose.pose.position.y;
            tf2::Quaternion q(
                        odom->pose.pose.orientation.x,
                        odom->pose.pose.orientation.y,
                        odom->pose.pose.orientation.z,
                        odom->pose.pose.orientation.w
                        );
            tf2::Matrix3x3 mat(q);
            double roll, pitch, yaw;
            mat.getRPY(roll, pitch, yaw);
            // RCLCPP_INFO(m_rosNode->get_logger(), "Поза: (%.2f, %.2f, %.2f)", x, y, yaw);
            // boost::asio::post(m_ioContext, [x, y, yaw, this] {
            //     m_positionChangedCallback(x, y, yaw);
            // });
        });
        auto sub_map = m_rosNode->create_subscription<nav_msgs::msg::OccupancyGrid>
                ("/map", rclcpp::QoS(rclcpp::KeepLast(1)).transient_local(),
                 [this](const nav_msgs::msg::OccupancyGrid::SharedPtr map) {
            boost::asio::post(m_ioContext, [map = std::move(map), this] {
                m_mapUpdatedCallback(map);
            });
        });
        auto sub_vel = m_rosNode->create_subscription<geometry_msgs::msg::Twist>
                ("/cmd_vel", rclcpp::QoS(rclcpp::KeepLast(10)),
                 [this](const geometry_msgs::msg::Twist::SharedPtr twist) {
            double linear = twist->linear.x;
            double angular = twist->angular.z;
            boost::asio::post(m_ioContext, [linear, angular, this] {
                m_cmdVelCallback(linear, angular);
            });
        });
        rclcpp::spin(m_rosNode);
    });

    m_tfTimer.async_wait(std::bind_front(&ROSbridge::tfPoll, this));
}

ROSbridge::~ROSbridge() {
    m_rosThread.join();
}

void ROSbridge::shutdown() {
    m_tfTimer.cancel();
}

void ROSbridge::addWaypoint(double x, double y, double yaw) {
    RCLCPP_INFO(m_rosNode->get_logger(), "Target: %.2f, %.2f, %.2f", x, y, yaw);
    // Ожидание доступности сервера
    if (!m_navigateActionClient->wait_for_action_server(std::chrono::seconds(10))) {
        RCLCPP_ERROR(m_rosNode->get_logger(), "Action server недоступен");
        return;
    }

    // Формирование цели
    auto goal_msg = nav2_msgs::action::NavigateToPose::Goal();
    goal_msg.pose.header.frame_id = "map"; // или "odom" для режима без карты
    goal_msg.pose.header.stamp = m_rosNode->now();
    goal_msg.pose.pose.position.x = x;
    goal_msg.pose.pose.position.y = y;
    // Yaw -> кватернион (только вокруг Z)
    goal_msg.pose.pose.orientation.w = std::cos(yaw / 2.0);
    goal_msg.pose.pose.orientation.z = std::sin(yaw / 2.0);

    // Настройка колбэков для обратной связи
    auto send_goal_options = rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SendGoalOptions();

    send_goal_options.feedback_callback =
            [this](rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr, const std::shared_ptr<const nav2_msgs::action::NavigateToPose::Feedback> fb) {
        RCLCPP_INFO(m_rosNode->get_logger(), "Текущая позиция: x: %.2f, y: %.2f, yaw: %.2f",
                    fb->current_pose.pose.position.x, fb->current_pose.pose.position.y, fb->current_pose.pose.orientation.w);
        RCLCPP_INFO(m_rosNode->get_logger(), "Осталось: %.2f м", fb->distance_remaining);
    };

    send_goal_options.result_callback =
            [this](const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult & result) {
        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
            RCLCPP_INFO(m_rosNode->get_logger(), "Цель достигнута!");
        } else {
            RCLCPP_WARN(m_rosNode->get_logger(), "Задача не выполнена");
        }
    };

    // Отправка цели
    m_navigateActionClient->async_send_goal(goal_msg, send_goal_options);
}

void ROSbridge::plan(double x, double y, double yaw) {
    if (!m_planActionClient->action_server_is_ready()) {
        RCLCPP_WARN(m_rosNode->get_logger(), "Planner server ещё не готов");
        return;
    }

    auto goal_msg = nav2_msgs::action::ComputePathToPose::Goal();
    goal_msg.start.header.frame_id = "map";
    goal_msg.start.header.stamp = m_rosNode->now();
    goal_msg.start.pose.orientation.w = 0.0;
    goal_msg.start.pose.position.x = 0.0;
    goal_msg.start.pose.position.y = 0.0;
    goal_msg.goal = goal_msg.start;
    goal_msg.goal.pose.position.x = x;
    goal_msg.goal.pose.position.y = y;
    goal_msg.planner_id = "GridBased";
    goal_msg.use_start = false;

    auto options = rclcpp_action::Client<nav2_msgs::action::ComputePathToPose>::SendGoalOptions();

    options.goal_response_callback =
            [this](rclcpp_action::ClientGoalHandle<nav2_msgs::action::ComputePathToPose>::SharedPtr goal_handle) {
        if (!goal_handle) {
            RCLCPP_ERROR(m_rosNode->get_logger(), "Goal отклонён");
        } else {
            RCLCPP_INFO(m_rosNode->get_logger(), "Goal принят, планируем...");
        }
    };

    options.result_callback = [this](
            const rclcpp_action::ClientGoalHandle<nav2_msgs::action::ComputePathToPose>::WrappedResult &result) {
        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
            const auto &path = result.result->path;
            RCLCPP_INFO(m_rosNode->get_logger(), "Путь: %zu точек, время %.3f с",
                        path.poses.size(),
                        result.result->planning_time.sec +
                        result.result->planning_time.nanosec * 1e-9);

            std::vector<float> points;
            for (auto &pose : path.poses) {
                points.push_back(pose.pose.position.x);
                points.push_back(pose.pose.position.y);
            }
            m_planChangedCallback(points);
        }
    };

    m_planActionClient->async_send_goal(goal_msg, options);
}

void ROSbridge::tfPoll(const boost::system::error_code &error_code) {
    if (error_code) return;

    try {
        geometry_msgs::msg::TransformStamped t = m_tfBuffer.lookupTransform("map", "base_link", tf2::TimePointZero);
        double x = t.transform.translation.x;
        double y = t.transform.translation.y;
        tf2::Quaternion q(
                    t.transform.rotation.x,
                    t.transform.rotation.y,
                    t.transform.rotation.z,
                    t.transform.rotation.w
                    );
        tf2::Matrix3x3 mat(q);
        double roll, pitch, yaw;
        mat.getRPY(roll, pitch, yaw);
        // RCLCPP_INFO(m_rosNode->get_logger(), "Позиция: (%.2f, %.2f, %.2f)", x, y, yaw);
        boost::asio::post(m_ioContext, [x, y, yaw, this] {
            m_positionChangedCallback(x, y, yaw);
        });
    } catch (const tf2::TransformException &ex) {
        RCLCPP_WARN(m_rosNode->get_logger(), "Не удалось получить tf: %s", ex.what());
    }

    m_tfTimer.expires_at(m_tfTimer.expiry() + std::chrono::milliseconds(100));
    m_tfTimer.async_wait(std::bind_front(&ROSbridge::tfPoll, this));
}
