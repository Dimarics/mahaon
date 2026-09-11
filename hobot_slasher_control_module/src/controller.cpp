#include "controller.h"
#include <mrpt/poses/CPose3D.h>
#include <mrpt/kinematics/CVehicleVelCmd_DiffDriven.h>
#include <QTimer>

Controller::Controller()
    : m_mowingActive(false)
    , m_currentX(0)
    , m_currentY(0)
    , m_currentYaw(0)
    , m_currentVelX(0)
    , m_currentVelY(0)
    , m_currentVelYaw(0)
    , m_currentTime(mrpt::system::InvalidTimeStamp())
    , m_currentPointCloud(nullptr)
    , m_moveController(new QSerialPort(this))
    , m_webSocketServer(new QWebSocketServer("Hobot Slasher", QWebSocketServer::NonSecureMode, this))
    , m_lidar(new Lidar(this))
    , m_mapper(new Mapper(this))
    , m_navigator(*this)
{
    //================== Lidar =========================

    connect(m_lidar, &Lidar::scanChanged, this, &Controller::processScan);
    connect(m_lidar, &Lidar::pointCloudChanged,
            this, [this](const mrpt::obs::CObservationPointCloud::Ptr &pointCloud) {
        m_currentPointCloud = pointCloud;
        auto points_map = pointCloud->pointcloud;
        if (points_map && !points_map->empty()) {
            size_t size = points_map->size();
            QList<QPointF> scan_points(size);
            for (size_t i = 0; i < size; ++i) {
                QPointF point;
                points_map->getPoint(i, point.rx(), point.ry());
                scan_points[i] = point;
            }
            emit scanPointsChanged(scan_points);
        }
    });

    //================== Move Controller ===============

    m_moveController->setBaudRate(9600);
    m_moveController->setPortName("/dev/ttyAMA0");
    qDebug() << "open move controller port" << m_moveController->open(QIODevice::ReadWrite);

    //================== WebSocket =====================

    m_webSocketServer->listen(QHostAddress::Any, 8081);
    connect(m_webSocketServer, &QWebSocketServer::newConnection, this, [this] {
        if (!m_webSocketServer->hasPendingConnections()) return;
        QWebSocket *socket = m_webSocketServer->nextPendingConnection();
        socket->setParent(this);
        qDebug() << "new connection:" << socket;

        if (socket->requestUrl().path() == "/lidarMapViewer") {
            connect(m_mapper, &Mapper::mapUpdated, socket, [socket, this](const QByteArray &map) {
                sendMessage(map, socket);
            });
        } else if (socket->requestUrl().path() == "/lidarViewer") {
            connect(this, &Controller::scanPointsChanged, socket, [socket, this](const QList<QPointF> &scan_points) {
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << scan_points;
                sendMessage(data, socket);
            });
        } else {
            connect(m_mapper, &Mapper::positionChanged, socket, [socket, this](double x, double y, double yaw) {
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << MSG_ID_LOCAL_POSITION_CHANGED << float(x) << float(y) << float(yaw);
                sendMessage(data, socket);
            });
            connect(this, &Controller::velocitiesChanged, socket, [socket, this](double vx, double vy, double v_yaw) {
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << MSG_ID_VELOCITIES_CHANGED << float(vx) << float(vy) << float(v_yaw);
                sendMessage(data, socket);
            });
            // connect(m_mapper, &Mapper::cellPositionChanged, socket, [socket, this](const QPoint &cellPos) {
            //     QByteArray data;
            //     QDataStream stream(&data, QIODevice::WriteOnly);
            //     stream << MSG_ID_MAP_CELL_POSITION_CHANGED << cellPos;
            //     sendMessage(data, socket);
            // });
            connect(socket, &QWebSocket::binaryMessageReceived, socket, [this](const QByteArray &msg) {
                uint8_t function_code;
                QDataStream stream(msg);
                stream >> function_code;
                switch(function_code) {
                case MSG_ID_SET_MOVE_PARAMS: {
                    // if (m_mowingActive) break;
                    uint16_t stick_1_x, stick_1_y, stick_2_x, stick_2_y;
                    stream >> stick_1_x >> stick_1_y >> stick_2_x >> stick_2_y;
                    // qDebug() << "=========================";
                    // qDebug() << "stick_1_x: " << stick_1_x;
                    // qDebug() << "stick_1_y: " << stick_1_y;
                    // qDebug() << "stick_2_x: " << stick_2_x;
                    // qDebug() << "stick_2_y: " << stick_2_y;
                    setMoveParams(stick_1_x, stick_1_y, stick_2_x, stick_2_y);
                    break;
                }
                case MSG_ID_SET_MOTOR_SPEEDS: {
                    uint8_t left_motor_pwm, right_motor_pwm;
                    stream >> left_motor_pwm >> right_motor_pwm;
                    // qDebug() << left_motor_pwm << right_motor_pwm;
                    setMotorSpeeds(left_motor_pwm, right_motor_pwm);
                    break;
                }
                case MSG_ID_SET_WAYPOINT: {
                    bool relative;
                    float x, y, yaw;
                    stream >> x >> y >> yaw >> relative;
                    m_navigator.addWaypoint(x, y, yaw, relative);

                    //
                    // float _x = m_currentX + x * cos(m_currentYaw) - y * sin(m_currentYaw);
                    // float _y = m_currentY + x * sin(m_currentYaw) + y * cos(m_currentYaw);
                    // m_navigator.addWaypoint(_x, _y, yaw, relative);
                    break;
                }
                case MSG_ID_START_BUILDING_MAP:
                    m_mapper->startBuildingMap();
                    break;
                case MSG_ID_FINISH_BUILDING_MAP:
                    m_mapper->finishBuildingMap();
                    break;
                case MSG_ID_MOWING_ENABLE:
                    m_mowingActive = true;
                    // qDebug() << m_mowingActive;
                    break;
                case MSG_ID_MOWING_DISABLE:
                    m_mowingActive = false;
                    // qDebug() << m_mowingActive;
                    break;
                default:
                    qDebug() << "неизвестный код функции:" << function_code;
                    break;
                }
            });
        }
        connect(socket, &QWebSocket::disconnected, socket, [socket] {
            qDebug() << socket << "disconnected";
            socket->deleteLater();
        });
        // connect(this, &Controller::scanPointsChanged, socket, [socket](const QList<QPointF> &scan_points) {
        //     QByteArray data;
        //     QDataStream stream(&data, QIODevice::WriteOnly);
        //     stream << MSG_ID_SCAN_POINTS_CHANGED << scan_points;
        //     // qDebug() << scan_points.size() << data.size();
        //     if (socket->bytesToWrite() < 1'048'576 && socket->state() == QAbstractSocket::ConnectedState) {
        //         socket->sendBinaryMessage(data);
        //         socket->flush();
        //     }
        // });
    });

    //================== MAPPER ========================
    connect(m_mapper, &Mapper::positionChanged, this, [this](double x, double y, double yaw) {
        mrpt::system::TTimeStamp new_stamp = mrpt::Clock::now();
        if (m_currentTime == mrpt::system::InvalidTimeStamp()) {
            m_currentTime = new_stamp;
            return;
        }
        double dt = mrpt::system::timeDifference(m_currentTime, new_stamp);
        if (dt < 1e-9) return;
        m_currentTime = new_stamp;
        m_currentVelX = (x - m_currentX) / dt;
        m_currentVelY = (y - m_currentY) / dt;
        m_currentVelYaw = mrpt::math::angDistance(yaw, m_currentYaw) / dt;
        m_currentX = x;
        m_currentY = y;
        m_currentYaw = yaw;
        emit velocitiesChanged(m_currentVelX, m_currentVelY, m_currentVelYaw);
        // qDebug() << "========================";
        // qDebug() << "velocity x:" << m_currentVelX
        //          << "velocity y:"<< m_currentVelY
        //          << "velocity z:"<< m_currentVelZ;
        // qDebug() << dt;
        m_navigator.update();
    });
}

bool Controller::getCurrentPoseAndSpeeds(mrpt::math::TPose2D &curPose,
                                         mrpt::math::TTwist2D &curVelGlobal,
                                         mrpt::system::TTimeStamp &timestamp,
                                         mrpt::math::TPose2D &curOdometry,
                                         std::string &frame_id)
{
    curPose = mrpt::math::TPose2D(m_currentX, m_currentY, m_currentYaw);
    curVelGlobal = mrpt::math::TTwist2D(m_currentVelX, m_currentVelY, m_currentVelYaw);
    timestamp = mrpt::Clock::now();
    curOdometry = curPose;
    frame_id = "map";
    return true;
}

bool Controller::changeSpeeds(const mrpt::kinematics::CVehicleVelCmd &vel_cmd) {
    // Проверяем тип команды
    const auto* cmd = dynamic_cast<const mrpt::kinematics::CVehicleVelCmd_DiffDriven*>(&vel_cmd);
    if (!cmd) {
        std::cerr << "Ошибка: Неподдерживаемый тип команды!" << std::endl;
        return false;
    }

    // Преобразуем в PWM для левого и правого колеса
    const double maxLinearSpeed = 0.3;
    const double wheelBase = 0.162; // расстояние между колёсами

    double left_speed  = cmd->lin_vel - cmd->ang_vel * (wheelBase / 2.0);
    double right_speed = cmd->lin_vel + cmd->ang_vel * (wheelBase / 2.0);

    // Преобразуем скорости колес в PWM
    uint8_t left_pwm = (90.0 + std::clamp(left_speed / maxLinearSpeed, -1.0, 1.0) * 90.0);
    uint8_t right_pwm = (90.0 + std::clamp(right_speed / maxLinearSpeed, -1.0, 1.0) * 90.0);

    // Отправляем PWM на реальный робот
    setMotorSpeeds(left_pwm, right_pwm);

    // Вывод для отладки
    // std::cout << "PWM: L=" << int(left_pwm) << ", R=" << int(right_pwm)
    //           << " (v=" << cmd->lin_vel << " м/с, ω=" << cmd->ang_vel << " рад/с)" << std::endl;

    return true;
}

bool Controller::stop(bool) {
    if (!m_moveController->isOpen()) return false;
    setMotorSpeeds(90, 90);
    return true;
}

bool Controller::senseObstacles(mrpt::maps::CSimplePointsMap &obstacles, mrpt::system::TTimeStamp &timestamp) {
    if (!m_currentPointCloud) return false;
    obstacles = *m_currentPointCloud->pointcloud.get();
    // obstacles = mrpt::maps::CSimplePointsMap();
    timestamp = m_currentPointCloud->timestamp;
    return true;
}

mrpt::kinematics::CVehicleVelCmd::Ptr Controller::getStopCmd() {
    auto cmd = mrpt::kinematics::CVehicleVelCmd_DiffDriven::Create();
    cmd->lin_vel = 0.0;
    cmd->ang_vel = 0.0;
    return cmd;
}

mrpt::kinematics::CVehicleVelCmd::Ptr Controller::getEmergencyStopCmd() {
    return getStopCmd();
}

void Controller::sendMessage(const QByteArray &msg, QWebSocket *socket) {
    if (socket->bytesToWrite() < 1'048'576 && socket->state() == QAbstractSocket::ConnectedState) {
        socket->sendBinaryMessage(msg);
        socket->flush();
    }
}

void Controller::setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y) {
    QByteArray packet(13, 0);
    packet[ 0] = 0xFF;
    packet[ 1] = 0xFF;
    packet[ 2] = MSG_ID_SET_MOVE_PARAMS;
    packet[ 3] = 8;
    packet[ 4] = char(stick_1_x >> 8); packet[ 5] = char(stick_1_x);
    packet[ 6] = char(stick_1_y >> 8); packet[ 7] = char(stick_1_y);
    packet[ 8] = char(stick_2_x >> 8); packet[ 9] = char(stick_2_x);
    packet[10] = char(stick_2_y >> 8); packet[11] = char(stick_2_y);
    packet[12] = 0;
    for (int i = 2; i < 12; ++i) packet[12] += packet[i];
    m_moveController->write(packet);
}

void Controller::setMotorSpeeds(uint8_t left_motor_pwm, uint8_t right_motor_pwm) {
    QByteArray packet(7, 0);
    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = MSG_ID_SET_MOTOR_SPEEDS;
    packet[3] = 2;
    packet[4] = left_motor_pwm;
    packet[5] = right_motor_pwm;
    packet[6] = packet[2] + packet[3] + packet[4] + packet[5];
    m_moveController->write(packet);
}

void Controller::processScan(const mrpt::obs::CObservation2DRangeScan::Ptr &obs) {
    // void Controller::processScan(const mrpt::obs::CObservationPointCloud::Ptr &obs) {
    // const auto points_map = obs->buildAuxPointsMap<mrpt::maps::CPointsMap>();
    // auto points_map = obs->pointcloud;
    // if (points_map && !points_map->empty()) {
    //     size_t size = points_map->size();
    //     // qDebug() << "points map size:" << size;
    //     QList<QPointF> scan_points(size);
    //     for (size_t i = 0; i < size; ++i) {
    //         QPointF point;
    //         points_map->getPoint(i, point.rx(), point.ry());
    //         scan_points[i] = point;
    //     }
    //     emit scanPointsChanged(scan_points);
    // }

    //======================================================================

    m_mapper->processScan(obs);

    //======================================================================

    // тестовая езда
    // if (m_mowingActive) {
    //     uint16_t rotate = 700;
    //     for (int i = 0; i < scan->size; ++i) {
    //         bool valid = scan->points[i].range >= m_lidar->minRange() && scan->points[i].range <= m_lidar->maxRange();
    //         if (valid && scan->points[i].range < 0.30) {
    //             if ((scan->points[i].angle < 0 && scan->points[i].angle > mrpt::DEG2RAD(-45)) ||
    //                     (scan->points[i].angle >= 0 && scan->points[i].angle < mrpt::DEG2RAD(45))) {
    //                 rotate = 0;
    //             }
    //         }
    //     }
    //     setMoveParams(rotate, 200, 512, 512);
    // }
}
