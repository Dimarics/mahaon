#include "mahaon/rosbridge.h"
#include <boost/asio/post.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <nav_msgs/msg/path.hpp>

ROSbridge::ROSbridge(boost::asio::io_context &io_context)
// : node()(std::make_shared<rclcpp::Node>("mahaon"))
    : m_laserScanPublisher(node()->create_publisher<sensor_msgs::msg::LaserScan>("scan", rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile()))
    , m_navigateActionClient(rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(node(), "navigate_to_pose"))
    , m_planActionClient(rclcpp_action::create_client<nav2_msgs::action::ComputePathToPose>(node(), "compute_path_to_pose"))
    , m_saveMapClient(node()->create_client<slam_toolbox::srv::SerializePoseGraph>("slam_toolbox/serialize_map"))
    , m_ioContext(io_context)
    , m_tfBroadcaster(std::make_shared<tf2_ros::TransformBroadcaster>(node()))
    , m_tfBuffer(node()->get_clock())
    , m_tfListener(m_tfBuffer)
    , m_tfPollTimer(io_context, std::chrono::milliseconds(100))
{
    m_rosThread = std::thread([this] {
        auto sub_scan = node()->create_subscription<sensor_msgs::msg::LaserScan>
                ("/scan", rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile(),
                 [this](const sensor_msgs::msg::LaserScan::SharedPtr laser_scan) {
            boost::asio::post(m_ioContext, [laser_scan = std::move(laser_scan), this] {
                m_scanReceivedCallback(laser_scan);
            });
        });
        auto sub_odom = node()->create_subscription<nav_msgs::msg::Odometry>
                ("/odom", rclcpp::QoS(rclcpp::KeepLast(10)).reliable().transient_local(),
                 [this](const nav_msgs::msg::Odometry::SharedPtr odom) {
            double stamp = rclcpp::Time(odom->header.stamp).seconds();
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
            // RCLCPP_INFO(node()->get_logger(), "Поза: (%.2f, %.2f, %.2f)", x, y, yaw);


            // double vx = odom->twist.twist.linear.x;
            // double vy = odom->twist.twist.linear.y;
            // double vz = odom->twist.twist.angular.z;

            static double prev_x = 0;
            static double prev_y = 0;
            static double prev_th = 0;
            static double t = timestamp().seconds();
            double dx = x - prev_x;
            double dy = y - prev_y;
            double dth = yaw - prev_th;
            double dt = stamp - t;
            double vx = ( dx * cos(yaw) + dy * sin(yaw)) / dt;
            double vy = (-dx * sin(yaw) + dy * cos(yaw)) / dt;
            double vz = dth / dt;

            prev_x = x;
            prev_y = y;
            prev_th = yaw;
            t = stamp;

            boost::asio::post(m_ioContext, [stamp, x, y, yaw, vx, vy, vz, this] {
                m_laserOdomChangedCallback(stamp, x, y, yaw, vx, vy, vz);
            });
        });
        auto sub_map = node()->create_subscription<nav_msgs::msg::OccupancyGrid>
                ("/map", rclcpp::QoS(rclcpp::KeepLast(1)).transient_local(),
                 [this](const nav_msgs::msg::OccupancyGrid::SharedPtr map) {
            boost::asio::post(m_ioContext, [map = std::move(map), this] {
                m_mapUpdatedCallback(map);
            });
        });
        auto sub_vel = node()->create_subscription<geometry_msgs::msg::Twist>
                ("/cmd_vel", rclcpp::QoS(rclcpp::KeepLast(10)),
                 [this](const geometry_msgs::msg::Twist::SharedPtr twist) {
            double linear = twist->linear.x;
            double angular = twist->angular.z;
            boost::asio::post(m_ioContext, [linear, angular, this] {
                m_cmdVelCallback(linear, angular);
            });
        });

        auto static_tf_publisher = tf2_ros::StaticTransformBroadcaster(node());
        {
            geometry_msgs::msg::TransformStamped static_tf;
            static_tf.header.stamp = node()->now();
            static_tf.header.frame_id = "base_link";
            static_tf.child_frame_id = "laser";
            static_tf.transform.translation.x = 0;
            static_tf.transform.translation.y = 0;
            static_tf.transform.translation.z = 0;
            tf2::Quaternion q;
            q.setRPY(0, 0, 0);
            static_tf.transform.rotation.x = q.x();
            static_tf.transform.rotation.y = q.y();
            static_tf.transform.rotation.z = q.z();
            static_tf.transform.rotation.w = q.w();
            static_tf_publisher.sendTransform(static_tf);
        }
        rclcpp::spin(node());
    });

    m_tfPollTimer.async_wait(std::bind_front(&ROSbridge::tfPoll, this));
}

ROSbridge::~ROSbridge() {
    m_rosThread.join();
}

void ROSbridge::shutdown() {
    m_tfPollTimer.cancel();
}

void ROSbridge::publishScan(const sensor_msgs::msg::LaserScan::SharedPtr scan_msg) {
    m_laserScanPublisher->publish(*scan_msg);
}

void ROSbridge::publishTransform(double x, double y, double yaw) {
    geometry_msgs::msg::TransformStamped tf;
    tf.header.stamp = node()->now();
    tf.header.frame_id = "odom";
    tf.child_frame_id = "base_link";
    tf.transform.translation.x = x;
    tf.transform.translation.y = y;
    tf.transform.translation.z = 0;
    tf2::Quaternion q;
    q.setRPY(0, 0, yaw);
    tf.transform.rotation.x = q.x();
    tf.transform.rotation.y = q.y();
    tf.transform.rotation.z = q.z();
    tf.transform.rotation.w = q.w();
    m_tfBroadcaster->sendTransform(tf);
}

void ROSbridge::addWaypoint(double x, double y, double yaw) {
    RCLCPP_INFO(node()->get_logger(), "target: %.2f, %.2f, %.2f", x, y, yaw);

    if (!m_navigateActionClient->action_server_is_ready()) {
        RCLCPP_ERROR(node()->get_logger(), "action server недоступен");
        return;
    }

    // Формирование цели
    auto goal_msg = nav2_msgs::action::NavigateToPose::Goal();
    goal_msg.pose.header.frame_id = "map"; // или "odom" для режима без карты
    goal_msg.pose.header.stamp = node()->now();
    goal_msg.pose.pose.position.x = x;
    goal_msg.pose.pose.position.y = y;
    // Yaw -> кватернион (только вокруг Z)
    // goal_msg.pose.pose.orientation.w = std::cos(yaw / 2.0);
    // goal_msg.pose.pose.orientation.z = std::sin(yaw / 2.0);

    // Настройка колбэков для обратной связи
    auto send_goal_options = rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SendGoalOptions();

    send_goal_options.feedback_callback =
            [this](rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr,
            const std::shared_ptr<const nav2_msgs::action::NavigateToPose::Feedback> fb) {
        RCLCPP_INFO(node()->get_logger(), "текущая позиция: x: %.2f, y: %.2f, yaw: %.2f",
                    fb->current_pose.pose.position.x, fb->current_pose.pose.position.y, fb->current_pose.pose.orientation.w);
        RCLCPP_INFO(node()->get_logger(), "осталось: %.2f м", fb->distance_remaining);
    };

    send_goal_options.result_callback =
            [this](const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult & result) {
        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
            RCLCPP_INFO(node()->get_logger(), "цель достигнута!");
        } else {
            RCLCPP_WARN(node()->get_logger(), "задача не выполнена");
        }
    };

    // Отправка цели
    m_navigateActionClient->async_send_goal(goal_msg, send_goal_options);
}

void ROSbridge::plan(double x, double y, double yaw) {
    if (!m_planActionClient->action_server_is_ready()) {
        RCLCPP_WARN(node()->get_logger(), "planner server не готов");
        return;
    }

    auto goal_msg = nav2_msgs::action::ComputePathToPose::Goal();
    goal_msg.start.header.frame_id = "map";
    goal_msg.start.header.stamp = node()->now();
    // goal_msg.start.pose.orientation.w = 0.0;
    // goal_msg.start.pose.position.x = 0.0;
    // goal_msg.start.pose.position.y = 0.0;
    goal_msg.goal = goal_msg.start;
    goal_msg.goal.pose.position.x = x;
    goal_msg.goal.pose.position.y = y;
    goal_msg.planner_id = "GridBased";
    goal_msg.use_start = false;

    auto options = rclcpp_action::Client<nav2_msgs::action::ComputePathToPose>::SendGoalOptions();

    options.goal_response_callback =
            [this](rclcpp_action::ClientGoalHandle<nav2_msgs::action::ComputePathToPose>::SharedPtr goal_handle) {
        if (!goal_handle) {
            RCLCPP_ERROR(node()->get_logger(), "goal отклонён");
        } else {
            RCLCPP_INFO(node()->get_logger(), "goal принят, идёт планирование...");
        }
    };

    options.result_callback = [this](
            const rclcpp_action::ClientGoalHandle<nav2_msgs::action::ComputePathToPose>::WrappedResult &result) {
        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
            const auto &path = result.result->path;
            RCLCPP_INFO(node()->get_logger(), "путь: %zu точек, время %.3f с",
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

void ROSbridge::saveMap() {
    if (!m_saveMapClient->service_is_ready()) {
        RCLCPP_WARN(node()->get_logger(), "Save map сервис не готов");
        return;
    }
    auto request = std::make_shared<slam_toolbox::srv::SerializePoseGraph::Request>();
    request->filename = "/home/mes/maps/map";
    m_saveMapClient->async_send_request(request);
}

void ROSbridge::tfPoll(const boost::system::error_code &error_code) {
    if (error_code) return;

    auto sendTransform = [this](const std::string &target_frame, const std::string &source_frame) {
        auto tf = m_tfBuffer.lookupTransform(target_frame, source_frame, tf2::TimePointZero);
        double x = tf.transform.translation.x;
        double y = tf.transform.translation.y;
        tf2::Quaternion q(
                    tf.transform.rotation.x,
                    tf.transform.rotation.y,
                    tf.transform.rotation.z,
                    tf.transform.rotation.w
                    );
        tf2::Matrix3x3 mat(q);
        double roll, pitch, yaw;
        mat.getRPY(roll, pitch, yaw);
        boost::asio::post(m_ioContext, [x, y, yaw, this] {
            m_positionChangedCallback(x, y, yaw);
        });
        m_tfPollTimer.expires_at(m_tfPollTimer.expiry() + std::chrono::milliseconds(100));
    };
    if (m_tfBuffer.canTransform("map", "base_link", tf2::TimePointZero)) {
        sendTransform("map", "base_link");
    } else if (m_tfBuffer.canTransform("odom", "base_link", tf2::TimePointZero)) {
        sendTransform("odom", "base_link");
    } else {
        RCLCPP_WARN(node()->get_logger(), "не удалось получить tf");
        m_tfPollTimer.expires_at(m_tfPollTimer.expiry() + std::chrono::milliseconds(1000));
    }
    // try {
    //     geometry_msgs::msg::TransformStamped t = m_tfBuffer.lookupTransform("map", "base_link", tf2::TimePointZero);
    //     double x = t.transform.translation.x;
    //     double y = t.transform.translation.y;
    //     tf2::Quaternion q(
    //                 t.transform.rotation.x,
    //                 t.transform.rotation.y,
    //                 t.transform.rotation.z,
    //                 t.transform.rotation.w
    //                 );
    //     tf2::Matrix3x3 mat(q);
    //     double roll, pitch, yaw;
    //     mat.getRPY(roll, pitch, yaw);
    //     // RCLCPP_INFO(node()->get_logger(), "Позиция: (%.2f, %.2f, %.2f)", x, y, yaw);
    //     boost::asio::post(m_ioContext, [x, y, yaw, this] {
    //         m_positionChangedCallback(x, y, yaw);
    //     });
    //     m_tfPollTimer.expires_at(m_tfPollTimer.expiry() + std::chrono::milliseconds(100));
    // } catch (const tf2::TransformException &ex) {
    //     RCLCPP_WARN(node()->get_logger(), "не удалось получить tf: %s", ex.what());
    //     m_tfPollTimer.expires_at(m_tfPollTimer.expiry() + std::chrono::milliseconds(1000));
    // }
    m_tfPollTimer.async_wait(std::bind_front(&ROSbridge::tfPoll, this));
}
