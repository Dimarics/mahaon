#ifndef ROSBRIDGE_H
#define ROSBRIDGE_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <nav2_msgs/action/compute_path_to_pose.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <slam_toolbox/srv/serialize_pose_graph.hpp>
#include <tf2_ros/transform_broadcaster.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>

class ROSbridge
{
public:
    // using NavigateToPose = nav2_msgs::action::NavigateToPose;
    // using GoalHandle = rclcpp_action::ClientGoalHandle<NavigateToPose>;

    ROSbridge(boost::asio::io_context &io_context);
    ~ROSbridge();
    void shutdown();

    void publishTransform(double x, double y, double yaw);
    void addWaypoint(double x, double y, double yaw = 0.0);
    void plan(double x, double y, double yaw = 0.0);
    void saveMap();

    inline void onScanReceived(const std::function<void(const sensor_msgs::msg::LaserScan::SharedPtr)> &callback) {
        m_scanReceivedCallback = callback;
    }
    inline void onLaserOdomChanged(const std::function<void (double, double, double)> &callback) {
        m_laserOdomChangedCallback = callback;
    }
    inline void onPositionChanged(const std::function<void (double, double, double)> &callback) {
        m_positionChangedCallback = callback;
    }
    inline void onMapUpdated(const std::function<void (const nav_msgs::msg::OccupancyGrid::SharedPtr)> &callback) {
        m_mapUpdatedCallback = callback;
    }
    inline void onCmdVelReceived(const std::function<void (double, double)> &callback) {
        m_cmdVelCallback = callback;
    }
    inline void onPlanChanged(const std::function<void(const std::vector<float>&)> &callback) {
        m_planChangedCallback = callback;
    }

private:
    std::shared_ptr<rclcpp::Node> m_rosNode;
    boost::asio::io_context &m_ioContext;
    std::thread m_rosThread;
    rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr m_navigateActionClient;
    rclcpp_action::Client<nav2_msgs::action::ComputePathToPose>::SharedPtr m_planActionClient;
    rclcpp::Client<slam_toolbox::srv::SerializePoseGraph>::SharedPtr m_saveMapClient;

    std::function<void(const sensor_msgs::msg::LaserScan::SharedPtr)> m_scanReceivedCallback;
    std::function<void(double, double, double)> m_laserOdomChangedCallback;
    std::function<void(double, double, double)> m_positionChangedCallback;
    std::function<void(const nav_msgs::msg::OccupancyGrid::SharedPtr map)> m_mapUpdatedCallback;
    std::function<void(double, double)> m_cmdVelCallback;
    std::function<void(const std::vector<float>&)> m_planChangedCallback;

    // tf
    std::shared_ptr<tf2_ros::TransformBroadcaster> m_tfBroadcaster;
    tf2_ros::Buffer m_tfBuffer;
    tf2_ros::TransformListener m_tfListener;
    boost::asio::steady_timer m_tfTimer;
    void tfPoll(const boost::system::error_code &ec);
};


#endif // ROSBRIDGE_H
