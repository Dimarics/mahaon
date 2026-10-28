#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <functional>
#include <queue>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <chrono>

class Navigator
{
public:
    struct Point {
        Point(double _x = 0.0, double _y = 0.0, double _yaw = 0.0) : x(_x), y(_y), yaw(_yaw) { }
        double x;
        double y;
        double yaw;

        Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
        Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
        Point operator*(double s) const { return {x * s, y * s}; }

        double dot(const Point& o) const { return x * o.x + y * o.y; }
        double lengthSq() const { return x * x + y * y; }
        double distSq(const Point& b) const {
            double dx = x - b.x;
            double dy = y - b.y;
            return dx * dx + dy * dy;
        }
    };
    Navigator();
    void setWaypoint(double x, double y);
    void addWaypoint(double x, double y);
    void clearWaypoints();
    void updatePos(double x, double y, double yaw);
    void updateObstacles(const sensor_msgs::msg::LaserScan::SharedPtr &laser_scan);
    void updateMap(const nav_msgs::msg::OccupancyGrid::SharedPtr &map);

    inline bool goalReached() { return m_goalReached; }
    inline void onTwistChanged(const std::function<void (double, double)> &callback) {
        m_twistChangedCallback = callback;
    }
    inline void onWaypointsChanged(const std::function<void (const std::vector<Point>)> &callback) {
        m_waypointsChanged = callback;
    }
    inline void onPlanUpdateRequest(const std::function<void (const Point &goal)> &callback) {
        m_planUpdateRequest = callback;
    }

private:
    const double kp_ang;
    const double max_linear;
    const double max_angular;
    const double robot_radius;
    const double safe_distance;

    bool m_goalReached;
    Point m_pos;
    // Position m_targetPos;
    std::queue<Point> m_waypoints;
    std::vector<Point> m_mapObstacles;
    sensor_msgs::msg::LaserScan::SharedPtr m_laserScan;
    std::function<void (double, double)> m_twistChangedCallback;
    std::function<void (const std::vector<Point>)> m_waypointsChanged;
    std::function<void (const Point &goal)> m_planUpdateRequest;
    std::chrono::steady_clock::time_point m_timestamp;

    double angleDistance(double from, double to);
    std::vector<size_t> obstaclesOnWay(const Point& from, const Point& to);
    bool lineIntersectsCircle(const Point& A, const Point& B, const Point& C, double R);
    std::vector<Point> findConnectedCircles(const std::vector<size_t>& obtacles_on_way);
    Point minFarthestPoint(const Point& from, const Point& to, const std::vector<Point> &obtacles);
    void moveToPoint();
};

#endif // NAVIGATOR_H
