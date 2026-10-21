#include "mahaon/controller.h"
#include "mahaon/buf_datastream.h"

Controller::Controller(boost::asio::io_context &io_context)
    : m_rosBridge(io_context)
    , m_webServer(io_context, 8081)
    , m_motionController(io_context, "/dev/ttyAMA0")
    , m_imu(io_context)
{
    m_rosBridge.onScanReceived([this](const sensor_msgs::msg::LaserScan::SharedPtr laser_scan) {
        std::vector<uint8_t> buffer;
        BufWriter writer(buffer);
        writer << MSG_ID_SCAN_RECEIVED << laser_scan->ranges.size();
        for (size_t i = 0; i < laser_scan->ranges.size(); ++i) {
            float angle = laser_scan->angle_min + i * laser_scan->angle_increment;
            float x =  laser_scan->ranges.at(i) * cos(angle);
            float y = -laser_scan->ranges.at(i) * sin(angle);
            writer << x << y;
        }
        m_webServer.send(buffer);
        m_navigator.updateObstacles(laser_scan);
    });
    m_rosBridge.onPositionChanged([this](double x, double y, double yaw) {
        m_pos.x = x;
        m_pos.y = y;
        m_pos.yaw = yaw;
        m_webServer.send(MSG_ID_LOCAL_POSITION_CHANGED, float(x), float(y), float(yaw));
        // std::cout << "x: " << x << " y: " << y << " yaw: " << yaw << std::endl;
        // if (!m_goalReached) {
        //     moveToPoint();
        // }
        m_navigator.updatePos(x, y, yaw);
    });
    m_rosBridge.onMapUpdated([this](const nav_msgs::msg::OccupancyGrid::SharedPtr map) {
        std::vector<uint8_t> buffer;
        BufWriter writer(buffer);
        writer << MSG_ID_MAP_UPDATED
               << map->info.width << map->info.height << map->info.resolution
               << map->info.origin.position.x << map->info.origin.position.y
               << uint32_t(map->data.size());
        for (auto &cell : map->data) writer << cell;
        m_webServer.send(buffer);
        m_navigator.updateMap(map);
    });
    // m_rosBridge.onCmdVelReceived([this](double lin_vel, double ang_vel) {
    //     setTwist(lin_vel, ang_vel);
    // });
    m_rosBridge.onPlanChanged([this](const std::vector<float> &path) {
        std::vector<uint8_t> buffer;
        BufWriter writer(buffer);
        writer << MSG_ID_WAYPOINTS_UPDATED << uint32_t(path.size() / 2);
        for (auto &point : path) writer << point;
        m_webServer.send(buffer);

        m_navigator.clearWaypoints();
        for (size_t i = 0; i < path.size(); i += 2) m_navigator.addWaypoint(path.at(i), path.at(i+1));
    });

    // server
    m_webServer.onMessageReceived([this](std::vector<uint8_t> &&msg) {
        // std::cout << "msg received" << std::endl;
        uint8_t cmd;
        BufReader reader(msg);
        reader >> cmd;
        switch (cmd) {
        case MSG_ID_SET_MOTOR_PWM: {
            int16_t left_motor_val, right_motor_val;
            reader >> left_motor_val >> right_motor_val;
            // std::cout << left_motor_val << " " << right_motor_val << std::endl;
            if (m_navigator.goalReached()) setMotorPwm(left_motor_val, right_motor_val);
            break;
        }
        case MSG_ID_SET_MOTOR_SPEEDS: {
            float lin_vel, ang_vel;
            reader >> lin_vel >> ang_vel;
            // std::cout << left_motor_val << " " << right_motor_val << std::endl;
            if (m_navigator.goalReached()) setTwist(lin_vel, ang_vel);
            break;
        }
        case MSG_ID_SET_WAYPOINT: {
            bool relative;
            float x, y, yaw;
            reader >> x >> y >> yaw >> relative;
            // std::cout << x << " " << y << " " << yaw << std::endl;

            // относительная позиция
            double target_x = x;
            double target_y = y;
            if (relative) {
                target_x = m_pos.x + x * cos(m_pos.yaw) - y * sin(m_pos.yaw);
                target_y = m_pos.y + x * sin(m_pos.yaw) + y * cos(m_pos.yaw);
            }
            // m_goalReached = false;
            // m_rosBridge.addWaypoint(target_x, target_y, yaw);
            // m_navigator.setWaypoint(target_x, target_y);
            m_rosBridge.plan(target_x, target_y, yaw);
            break;
        }
        case MSG_ID_ADD_WAYPOINT: {
            bool relative;
            float x, y, yaw;
            reader >> x >> y >> yaw >> relative;
            double target_x = x;
            double target_y = y;
            if (relative) {
                target_x = m_pos.x + x * cos(m_pos.yaw) - y * sin(m_pos.yaw);
                target_y = m_pos.y + x * sin(m_pos.yaw) + y * cos(m_pos.yaw);
            }
            m_navigator.addWaypoint(target_x, target_y);
            break;
        }
        case MSG_ID_CLEAR_WAYPOINTS: {
            m_navigator.clearWaypoints();
            break;
        }
        default:
            std::cout << "неизвестный код функции: " << uint16_t(cmd) << std::endl;
            break;
        }
    });

    // port
    // m_motionController.onDataReceived(std::bind_front(&PacketAssembler::addData, &m_motionControllerPacketAssembler));
    m_motionController.onDataReceived(std::bind_front(&PacketAssembler::addData, &m_motionControllerPacketAssembler));
    m_motionControllerPacketAssembler.onPacketAssembled([this](uint8_t cmd, const std::vector<uint8_t> &data) {
        std::vector<uint8_t> _data(data);
        BufReader reader(_data);
        switch(cmd) {
        case MSG_ID_ENC_CHANGED: {
            float dl, dr;
            reader >> dl >> dr;

            static float x = 0, y = 0, f = 0;
            float b = 0.16;
            float d = (dr + dl) / 2.0;
            float df = (dr - dl) / b;
            float R = d / df;

            if (df != 0) {
                x = x + R * (sin(f + df) - sin(f));
                y = y - R * (cos(f + df) - cos(f));
                f = f + df;
            } else {
                x += d * cos(f);
            }
            // std::vector<uint8_t> msg;
            // BufWriter writer(msg);
            // writer << MSG_ID_ENC_CHANGED << x << y << f;
            // std::cout << x << " " << y << std::endl;
            m_webServer.send(MSG_ID_ENC_CHANGED, x, y, f);

            break;
        }
        case MSG_ID_IMU_CHANGED: {
            break;
        }
        default:
            std::cout << "неизвестный код функции: " << uint16_t(cmd) << std::endl;
            break;
        }
    });

    // навигатор
    m_navigator.onTwistChanged(std::bind_front(&Controller::setTwist, this));
    m_navigator.onWaypointsChanged([this](const std::vector<Navigator::Point> &waypoints) {
        std::vector<uint8_t> buffer;
        BufWriter writer(buffer);
        writer << MSG_ID_WAYPOINTS_UPDATED << uint32_t(waypoints.size());
        for (auto &waypoint : waypoints) writer << float(waypoint.x) << float(waypoint.y);
        m_webServer.send(buffer);
    });
}

void Controller::setTwist(double lin_vel, double ang_vel)
{
    const double wheel_base = 0.16; // расстояние между колёсами
    // lin_vel = 0.1;
    // ang_vel = 0.5;

    double left_speed  = lin_vel - ang_vel * wheel_base / 2.0;
    double right_speed = lin_vel + ang_vel * wheel_base / 2.0;

    // Преобразуем скорости колес в PWM
    constexpr double to_rpm = 60.0 / (M_PI * 0.066);
    constexpr double to_pwm = 4096.0 / 60.0;
    int16_t left_motor = left_speed * to_rpm * to_pwm;
    int16_t right_motor = right_speed * to_rpm * to_pwm;
    std::cout << left_motor << " " << right_motor << std::endl;

    // Отправляем PWM на реальный робот
    setMotorPwm(left_motor, right_motor);
}

void Controller::setMotorPwm(int16_t left_motor, int16_t right_motor) {
    std::vector<uint8_t> msg;
    PacketBuilder packetBuilder(msg);
    packetBuilder.begin();
    packetBuilder << MSG_ID_SET_MOTOR_SPEEDS << left_motor << right_motor;
    packetBuilder.end();
    m_motionController.send(msg);
}

void Controller::shutdown() {
    m_rosBridge.shutdown();
    m_webServer.shutdown();
    m_motionController.shutdown();
    m_imu.shutdown();
}
