#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <functional>
#include <queue>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <chrono>
#include <mahaon/vec.h>

class Navigator
{
public:
    struct Position {
        double x;
        double y;
        double yaw;
    };
    Navigator();
    void setWaypoint(double x, double y);
    void addWaypoint(double x, double y);
    void clearWaypoints();
    void updatePos(double x, double y, double yaw);
    void updateObstacles(const sensor_msgs::msg::LaserScan::SharedPtr &laser_scan);
    void updateMap(const nav_msgs::msg::OccupancyGrid::SharedPtr &map);
    void setTwist(double lin_vel, double ang_vel);

    inline bool goalReached() { return m_goalReached; }
    // inline void onTwistChanged(const std::function<void (double, double)> &callback) {
    //     m_twistChangedCallback = callback;
    // }
    inline void onPwmChanged(const std::function<void (int32_t, int32_t)> &callback) {
        m_pwmChangedCallback = callback;
    }
    inline void onWaypointsChanged(const std::function<void (const std::vector<Position>)> &callback) {
        m_waypointsChanged = callback;
    }
    inline void onPlanUpdateRequest(const std::function<void (const Position &goal)> &callback) {
        m_planUpdateRequest = callback;
    }

private:
    double _robot_radius;
    double _wheel_base;
    double _wheel_d;
    double _max_lin_vel;
    double _max_ang_vel;
    double _k_pwm;
    double _kp_lin;
    double _kp_ang;
    double _influence_r;
    double _safe_distance;
    double _reach_distance;
    double _cost_scaling_factor;

    bool m_goalReached;
    Position m_pos;
    // Position m_targetPos;
    std::queue<Position> m_waypoints;
    std::vector<VecF> m_mapObstacles;
    sensor_msgs::msg::LaserScan::SharedPtr m_laserScan;
    // std::function<void (double, double)> m_twistChangedCallback;
    std::chrono::steady_clock::time_point m_timestamp;

    std::function<void (int32_t, int32_t)> m_pwmChangedCallback;
    std::function<void (const std::vector<Position>)> m_waypointsChanged;
    std::function<void (const Position &goal)> m_planUpdateRequest;

    double angleDistance(double from, double to);
    std::vector<size_t> obstaclesOnWay(const VecF& from, const VecF& to);
    bool lineIntersectsCircle(const VecF& A, const VecF& B, const VecF& C, double R);
    std::vector<VecF> findConnectedCircles(const std::vector<size_t>& obtacles_on_way);
    VecF minFarthestPoint(const VecF& from, const VecF& to, const std::vector<VecF> &obtacles);
    void moveToPoint();
};

#endif // NAVIGATOR_H
