#include "mahaon/navigator.h"
#include <algorithm>
#include <cmath>

Navigator::Navigator()
    : kp_ang(1.2)
    , max_linear(0.15)
    , max_angular(1.0)
    , robot_radius(0.15)
    , safe_distance(0.2)

    , m_goalReached(true)
    , m_pos(0, 0)
    , m_laserScan(nullptr) { }

void Navigator::setWaypoint(double x, double y) {
    m_waypoints = std::queue<Point>();
    addWaypoint(x, y);
}

void Navigator::addWaypoint(double x, double y) {
    m_goalReached = false;
    Point back_point = m_pos;
    if (!m_waypoints.empty()) back_point = m_waypoints.back();
    auto object_points = findConnectedCircles(obstaclesOnWay(back_point, {x, y}));
    if (!object_points.empty()) {
        m_waypoints.push(minFarthestPoint(back_point, {x, y}, object_points));
    }
    m_waypoints.push({x, y});

    std::queue<Point> _waypoints = m_waypoints;
    std::vector<Point> waypoints;
    while(!_waypoints.empty()) {
        waypoints.push_back(_waypoints.front());
        _waypoints.pop();
    }
    m_waypointsChanged(waypoints);
}

void Navigator::clearWaypoints() {
    m_goalReached = true;
    m_waypoints = std::queue<Point>{};
}

void Navigator::updatePos(double x, double y, double yaw) {
    m_pos.x = x;
    m_pos.y = y;
    m_pos.yaw = yaw;
    if (!m_goalReached) {
        moveToPoint();
    }
}

void Navigator::updateObstacles(const sensor_msgs::msg::LaserScan::SharedPtr &laser_scan) {
    m_laserScan = laser_scan;
}

void Navigator::updateMap(const nav_msgs::msg::OccupancyGrid::SharedPtr &map) {
    m_mapObstacles.clear();
    for (uint32_t x = 0; x < map->info.width; ++x) {
        for (uint32_t y = 0; y < map->info.height; ++y) {
            // формула для доступа к ячейке: x + y * size_x
            const int8_t cell_data = uint8_t(map->data.at(x + y * map->info.width));
            if (cell_data == 100) {
                const double mx = map->info.origin.position.x + (x + 0.5) * map->info.resolution;
                const double my = map->info.origin.position.y + (y + 0.5) * map->info.resolution;
                m_mapObstacles.push_back(Point(mx, my));
            }
        }
    }
}

double Navigator::angleDistance(double from, double to) {
    double da = to - from;
    if (da > M_PI) da -= 2.0 * M_PI;
    if (da < -M_PI) da += 2.0 * M_PI;
    return da;
}

bool Navigator::lineIntersectsCircle(const Point &A, const Point &B, const Point &C, double R) {
    const double R2 = R * R;
    const Point CA = A - C;
    const Point CB = B - C;

    const double dA = CA.lengthSq();
    const double dB = CB.lengthSq();

    // Оба конца внутри окружности — пересечения нет
    if (dA < R2 && dB < R2) return false;

    // Хотя бы один конец внутри или на границе — пересечение есть
    if (dA <= R2 || dB <= R2) return true;

    // Оба конца снаружи: ищем ближайшую точку отрезка к центру
    const Point AB = B - A;
    const double abLenSq = AB.lengthSq();

    // Вырожденный отрезок (A == B): оба снаружи → нет пересечения
    if (abLenSq == 0.0) return false;

    // Параметр ближайшей точки на прямой, ограниченный отрезком [0, 1]
    double t = (C - A).dot(AB) / abLenSq;
    if (t < 0.0) t = 0.0;
    else if (t > 1.0) t = 1.0;

    const Point P = A + AB * t;      // ближайшая точка отрезка к центру
    const double dMin = (P - C).lengthSq();

    return dMin <= R2;
}

std::vector<size_t> Navigator::obstaclesOnWay(const Point &from, const Point &to) {
    std::vector<size_t> result;
    for (size_t i = 0; i < size_t(m_mapObstacles.size()); ++i) {
        if (lineIntersectsCircle(from, to, m_mapObstacles.at(i), safe_distance)) {
            result.push_back(i);
        }
    }
    return result;
}

std::vector<Navigator::Point> Navigator::findConnectedCircles(const std::vector<size_t> &obstacles_on_way) {
    const size_t n = m_mapObstacles.size();
    const double threshold = 4.0 * safe_distance * safe_distance;

    std::vector<bool> visited(n, false);
    std::vector<Point> result;
    std::queue<size_t> q;

    for (auto &index : obstacles_on_way) {
        visited[index] = true;
        q.push(index);
    }

    while (!q.empty()) {
        size_t cur = q.front();
        q.pop();
        result.push_back(m_mapObstacles.at(cur));

        for (size_t i = 0; i < n; ++i) {
            if (visited[i]) continue;
            if (m_mapObstacles[cur].distSq(m_mapObstacles[i]) <= threshold) {
                visited[i] = true;
                q.push(i);
            }
        }
    }
    return result;
}

Navigator::Point Navigator::minFarthestPoint(const Point& from, const Point& to, const std::vector<Point> &obstacles) {
    // Коэффициенты прямой ax + by + c = 0
    double a = to.y - from.y;
    double b = -(to.x - from.x);
    double c = -(a * from.x + b * from.y);

    double n = std::sqrt(a*a + b*b);
    if (n == 0.0) {
        return Point(0, 0); // m_from == m_to, вырожденный случай
    }

    double na = a / n, nb = b / n;

    double bestPlusDist  = -1.0; // максимум расстояния по положительной стороне
    double bestMinusDist = -1.0; // максимум расстояния по отрицательной стороне
    Point  bestPlus(0, 0);
    Point  bestMinus(0, 0);

    for (const auto &obs : obstacles) {
        // Две крайние точки этой окружности
        Point P1 = obs + Point{na * safe_distance, nb * safe_distance};
        Point P2 = obs - Point{na * safe_distance, nb * safe_distance};

        double f1 = a * P1.x + b * P1.y + c;
        double f2 = a * P2.x + b * P2.y + c;

        double d1 = std::abs(f1) / n;
        double d2 = std::abs(f2) / n;

        // Классифицируем по знаку f(P)
        if (f1 > 0) {
            if (d1 > bestPlusDist) { bestPlusDist = d1; bestPlus = P1; }
        } else if (f1 < 0) {
            if (d1 > bestMinusDist) { bestMinusDist = d1; bestMinus = P1; }
        }

        if (f2 > 0) {
            if (d2 > bestPlusDist) { bestPlusDist = d2; bestPlus = P2; }
        } else if (f2 < 0) {
            if (d2 > bestMinusDist) { bestMinusDist = d2; bestMinus = P2; }
        }
    }

    if (bestPlusDist < 0 && bestMinusDist < 0) {
        return Point(0, 0); // нет окружностей
    }
    if (bestPlusDist < 0)  return bestMinus; // все окружности с одной стороны
    if (bestMinusDist < 0) return bestPlus;

    return (bestPlusDist <= bestMinusDist) ? bestPlus : bestMinus;
}

void Navigator::moveToPoint() {
    const double dx = m_waypoints.front().x - m_pos.x;
    const double dy = m_waypoints.front().y - m_pos.y;
    // const double dx = m_targetPos.x - m_pos.x;
    // const double dy = m_targetPos.y - m_pos.y;
    const double distance = std::hypot(dx, dy);
    // std::cout << "=====================================" << std::endl;
    // std::cout << "target distance: " << distance
    //           << "; target x: " << m_targetPos.x
    //           << "; target y: " << m_targetPos.y << std::endl;

    if (distance < robot_radius) {
        m_waypoints.pop();
        if (m_waypoints.empty()) {
            m_goalReached = true;
            m_twistChangedCallback(0, 0);
        }
        return;
    }

    double target_angle = std::atan2(dy, dx);
    double angle_error = angleDistance(m_pos.yaw, target_angle);

    // // проверка на столкновения
    // if (m_laserScan) {
    //     for (size_t i = 0; i < m_laserScan->ranges.size(); ++i) {
    //         if (m_laserScan->ranges.at(i) > 0 && m_laserScan->ranges.at(i) <= 0.3) {
    //             float angle = m_laserScan->angle_min + i * m_laserScan->angle_increment;
    //             if (std::abs(angle) < M_PI / 4.f) {
    //                 m_twistChangedCallback(0, 0.2);
    //                 return;
    //             }
    //         }
    //     }
    //     //
    //     for (size_t i = 0; i < m_laserScan->ranges.size(); ++i) {
    //         float p_range = m_laserScan->ranges.at(i);
    //         if (p_range > 0 && p_range <= 0.3) {
    //             const float p_angle = m_laserScan->angle_min + i * m_laserScan->angle_increment;
    //             const float dth = p_angle - angle_error;
    //             const float along = p_range * cos(dth);
    //             if (along <= 0 || along >= distance) {
    //                 continue;
    //             }
    //             const float prep = p_range * std::abs(sin(dth));
    //             if (prep < 0.3) {
    //                 m_twistChangedCallback(0.15, 0);
    //                 return;
    //             }
    //         }
    //     }
    // }

    double lin_vel = std::min(max_linear, distance);
    double ang_vel = std::clamp(kp_ang * angle_error, -max_angular, max_angular);

    if (std::abs(angle_error) > 20.0 / 180.0 * M_PI) {
        lin_vel = 0.0;
    }

    m_twistChangedCallback(lin_vel, ang_vel);
}
