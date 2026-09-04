#include "controller.h"
#include <mrpt/poses/CPose3D.h>

Controller::Controller()
    : m_mowingActive(false)
    , m_lidar(new Lidar(this))
    , m_mapper(new Mapper(this))
    , m_moveController(new QSerialPort(this))
    , m_webSocketServer(new QWebSocketServer("Hobot Slasher", QWebSocketServer::NonSecureMode, this))
{
    //================== Lidar =========================

    connect(m_lidar, &Lidar::scanChanged, this, &Controller::processScan);

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
            connect(m_mapper, &Mapper::cellPositionChanged, socket, [socket, this](const QPoint &cellPos) {
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << MSG_ID_MAP_CELL_POSITION_CHANGED << cellPos;
                sendMessage(data, socket);
            });
            connect(socket, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &msg) {
                uint8_t function_code;
                QDataStream stream(msg);
                stream >> function_code;
                switch(function_code) {
                case MSG_ID_SET_MOVE_PARAMS: {
                    if (m_mowingActive) break;
                    uint16_t stick_1_x, stick_1_y, stick_2_x, stick_2_y;
                    stream >> stick_1_x >> stick_1_y >> stick_2_x >> stick_2_y;
                    // qDebug() << "=========================";
                    // qDebug() << "stick_1_x: " << stick_1_x;
                    // qDebug() << "stick_1_y: " << stick_1_y;
                    // qDebug() << "stick_2_x: " << stick_2_x;
                    // qDebug() << "stick_2_y: " << stick_2_y;

                    // qDebug() << stick_1_x << stick_1_y << stick_2_x << stick_2_y;
                    setMoveParams(stick_1_x, stick_1_y, stick_2_x, stick_2_y);
                }
                    break;
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
                    qDebug() << "неизвестный код функции";
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
}

void Controller::sendMessage(const QByteArray &msg, QWebSocket *socket)
{
    if (socket->bytesToWrite() < 1'048'576 && socket->state() == QAbstractSocket::ConnectedState) {
        socket->sendBinaryMessage(msg);
        socket->flush();
    }
}

void Controller::setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y) {
    QByteArray packet(13, 0);
    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = MSG_ID_SET_MOVE_PARAMS;
    packet[3] = 8;
    packet[4] = char(stick_1_x >> 8); packet[5] = char(stick_1_x);
    packet[6] = char(stick_1_y >> 8); packet[7] = char(stick_1_y);
    packet[8] = char(stick_2_x >> 8); packet[9] = char(stick_2_x);
    packet[10] = char(stick_2_y >> 8); packet[11] = char(stick_2_y);
    // for (int i = 0; i < size; ++i) packet[i+4] += data[i];
    uint8_t crc = 0;
    for (int i = 2; i < packet.size(); ++i) crc += packet[i];
    packet[12] = crc;
    m_moveController->write(packet);
}

void Controller::processScan(const mrpt::obs::CObservation2DRangeScan::Ptr &obs) {
// void Controller::processScan(const mrpt::obs::CObservationPointCloud::Ptr &obs) {
    const auto points_map = obs->buildAuxPointsMap<mrpt::maps::CPointsMap>();
    // auto points_map = obs->pointcloud;
    if (points_map && !points_map->empty()) {
        size_t size = points_map->size();
        // qDebug() << "points map size:" << size;
        QList<QPointF> scan_points(size);
        for (size_t i = 0; i < size; ++i) {
            QPointF point;
            points_map->getPoint(i, point.rx(), point.ry());
            scan_points[i] = point;
        }
        emit scanPointsChanged(scan_points);
    }

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
