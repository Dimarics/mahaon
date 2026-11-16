#include "mahaon/navigator.h"
#include "mahaon/app_config.h"
#include <algorithm>
#include <cmath>
// #include <iostream>

Navigator::Navigator()
    : m_goalReached(true)
    , m_pos{0, 0, 0}
    , m_laserScan(nullptr)
    , m_pwmChangedCallback([](int32_t, int32_t){})
    , m_waypointsChanged([](const std::vector<Position>){})
    , m_planUpdateRequest([](const Position &goal){})
{
    YAML::Node vehicle_config = app_config::params["vehicle"];
    YAML::Node nav_config = app_config::params["vehicle"];
    _robot_radius        = vehicle_config["robot_radius"].as<double>(0.15);
    _wheel_base          = vehicle_config["wheel_base"].as<double>(1.0);
    _wheel_d             = vehicle_config["wheel_d"].as<double>(1.0);
    _max_lin_vel         = vehicle_config["max_linear_velocity"].as<double>(0.0);
    _max_ang_vel         = vehicle_config["max_angular_velocity"].as<double>(0.0) / 180.0 * M_PI;
    _k_pwm               = vehicle_config["k_pwm"].as<double>(1.0);
    _kp_lin              = nav_config["kp_lin"].as<double>(1.0);
    _kp_ang              = nav_config["kp_ang"].as<double>(1.0);
    _influence_r         = nav_config["influence_radius"].as<double>(_robot_radius * 5.0);
    _safe_distance       = nav_config["safe_distance"].as<double>(_robot_radius * 1.05);
    _reach_distance      = nav_config["reach_distance"].as<double>(_robot_radius * 0.3);
    _cost_scaling_factor = nav_config["cost_scaling_factor"].as<double>(8.0);
}

void Navigator::setWaypoint(double x, double y) {
    m_waypoints = std::queue<Position>();
    addWaypoint(x, y);
}

void Navigator::addWaypoint(double x, double y) {
    m_goalReached = false;
    // Position back_point = m_pos;
    // if (!m_waypoints.empty()) back_point = m_waypoints.back();
    // auto object_points = findConnectedCircles(obstaclesOnWay(back_point, {x, y}));
    // if (!object_points.empty()) {
    //     m_waypoints.push(minFarthestPoint(back_point, {x, y}, object_points));
    // }
    m_waypoints.push({x, y});

    std::queue<Position> _waypoints = m_waypoints;
    std::vector<Position> waypoints;
    while(!_waypoints.empty()) {
        waypoints.push_back(_waypoints.front());
        _waypoints.pop();
    }
    m_waypointsChanged(waypoints);
    m_timestamp = std::chrono::steady_clock::now();
}

void Navigator::clearWaypoints() {
    m_goalReached = true;
    m_waypoints = std::queue<Position>{};
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
                m_mapObstacles.push_back(VecF(mx, my));
            }
        }
    }
}

void Navigator::setTwist(double lin_vel, double ang_vel) {
    // VecF u = VecF(1, 0); // VecF u = VecF(lin_vel, 0).norm();

    if (m_laserScan) {
        double k_decel_min = 1.0;
        for (size_t i = 0; i < m_laserScan->ranges.size(); ++i) {
            double range = m_laserScan->ranges[i];
            if (m_laserScan->ranges[i] < m_laserScan->range_min || m_laserScan->ranges[i] > _influence_r ||
                    range > _influence_r)
                continue;
            double angle = m_laserScan->angle_min + i * m_laserScan->angle_increment;
            double c = cos(lin_vel > 0 ? angle : angle - M_PI); // cos(obs_ang - motion_ang)
            if (c > 0 && range < _safe_distance) {
                k_decel_min = 0.0;
                break;
            }
            double k_w = 0.8;
            double w_dir = k_w + (1.0 - k_w) * (1.0 + c) / 2.0;
            double cost = 1.0 - std::clamp(std::exp(-_cost_scaling_factor * (range - _safe_distance)), 0.0, 1.0);
            double k_decel = 1.0 - w_dir * (1.0 - cost);
            k_decel_min = std::min(k_decel, k_decel_min);
        }
        lin_vel *= k_decel_min;
    }

    double left_speed  = lin_vel - ang_vel * _wheel_base / 2.0;
    double right_speed = lin_vel + ang_vel * _wheel_base / 2.0;
    double to_rpm = 60.0 / (M_PI * _wheel_d);
    int16_t left_motor_pwm = left_speed * to_rpm * _k_pwm;
    int16_t right_motor_pwm = right_speed * to_rpm * _k_pwm;
    // std::cout << left_motor << " " << right_motor << std::endl;
    m_pwmChangedCallback(left_motor_pwm, right_motor_pwm);
}

double Navigator::angleDistance(double from, double to) {
    double da = to - from;
    if (da > M_PI) da -= 2.0 * M_PI;
    if (da < -M_PI) da += 2.0 * M_PI;
    return da;
}

bool Navigator::lineIntersectsCircle(const VecF &A, const VecF &B, const VecF &C, double R) {
    const double R2 = R * R;
    const VecF CA = A - C;
    const VecF CB = B - C;

    const double dA = CA.lengthSq();
    const double dB = CB.lengthSq();

    // Оба конца внутри окружности — пересечения нет
    if (dA < R2 && dB < R2) return false;

    // Хотя бы один конец внутри или на границе — пересечение есть
    if (dA <= R2 || dB <= R2) return true;

    // Оба конца снаружи: ищем ближайшую точку отрезка к центру
    const VecF AB = B - A;
    const double abLenSq = AB.lengthSq();

    // Вырожденный отрезок (A == B): оба снаружи → нет пересечения
    if (abLenSq == 0.0) return false;

    // Параметр ближайшей точки на прямой, ограниченный отрезком [0, 1]
    double t = (C - A).dot(AB) / abLenSq;
    if (t < 0.0) t = 0.0;
    else if (t > 1.0) t = 1.0;

    const VecF P = A + AB * t;      // ближайшая точка отрезка к центру
    const double dMin = (P - C).lengthSq();

    return dMin <= R2;
}

std::vector<size_t> Navigator::obstaclesOnWay(const VecF &from, const VecF &to) {
    std::vector<size_t> result;
    for (size_t i = 0; i < size_t(m_mapObstacles.size()); ++i) {
        if (lineIntersectsCircle(from, to, m_mapObstacles.at(i), _reach_distance)) {
            result.push_back(i);
        }
    }
    return result;
}

std::vector<VecF> Navigator::findConnectedCircles(const std::vector<size_t> &obstacles_on_way) {
    const size_t n = m_mapObstacles.size();
    const double threshold = 4.0 * _reach_distance * _reach_distance;

    std::vector<bool> visited(n, false);
    std::vector<VecF> result;
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

VecF Navigator::minFarthestPoint(const VecF& from, const VecF& to, const std::vector<VecF> &obstacles) {
    // Коэффициенты прямой ax + by + c = 0
    double a = to.y - from.y;
    double b = -(to.x - from.x);
    double c = -(a * from.x + b * from.y);

    double n = std::sqrt(a*a + b*b);
    if (n == 0.0) {
        return VecF(0, 0); // m_from == m_to, вырожденный случай
    }

    double na = a / n, nb = b / n;

    double bestPlusDist  = -1.0; // максимум расстояния по положительной стороне
    double bestMinusDist = -1.0; // максимум расстояния по отрицательной стороне
    VecF  bestPlus(0, 0);
    VecF  bestMinus(0, 0);

    for (const auto &obs : obstacles) {
        // Две крайние точки этой окружности
        VecF P1 = obs + VecF{na * _reach_distance, nb * _reach_distance};
        VecF P2 = obs - VecF{na * _reach_distance, nb * _reach_distance};

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
        return VecF(0, 0); // нет окружностей
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

    if (distance < _reach_distance) {
        m_waypoints.pop();
        if (m_waypoints.empty()) {
            m_goalReached = true;
            // m_twistChangedCallback(0, 0);
            setTwist(0, 0);
        }
        return;
    }

    double target_angle = std::atan2(dy, dx);
    double angle_error = angleDistance(m_pos.yaw, target_angle);

    // проверка на столкновения
    if (m_laserScan) {
        for (auto &range : m_laserScan->ranges) {
            if (range > m_laserScan->range_min && range < m_laserScan->range_max && range < _influence_r) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>
                        (std::chrono::steady_clock::now() - m_timestamp).count() > 800) {
                    m_timestamp = std::chrono::steady_clock::now();
                    m_planUpdateRequest(m_waypoints.back());
                }
            }
        }
    }

    double lin_vel = std::min(_kp_lin * distance, _max_lin_vel);
    double ang_vel = std::clamp(_kp_ang * angle_error, -_max_ang_vel, _max_ang_vel);

    if (std::abs(angle_error) > 20.0 / 180.0 * M_PI) {
        lin_vel = 0.0;
    }

    // m_twistChangedCallback(lin_vel, ang_vel);
    setTwist(lin_vel, ang_vel);
}
