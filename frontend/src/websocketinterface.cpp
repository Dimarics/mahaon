#include "websocketinterface.h"
#include "qbytearray_datastream.h"

WebSocketInterface::WebSocketInterface(QObject *parent)
    : QObject(parent)
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_connectTimer(new QTimer(this))
    , m_timeout(new QTimer(this))
{
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketInterface::messageHandler);
    // connect(m_webSocket, &QWebSocket::stateChanged, this, [](QAbstractSocket::SocketState state) {
    //     qDebug() << state;
    // });

    // connect(this, &QWebSocket::messageAccepted, this, [this] {
    //     //qDebug() << m_queue;
    //     if (!m_queue.isEmpty()) {
    //         m_queue.removeFirst();
    //         m_timeout->stop();
    //     }
    //     if (!m_queue.isEmpty()) {
    //         m_serialPort->write(m_queue.first());
    //         m_timeout->start();
    //     }
    //     //qDebug() << m_queue;
    // });

    // m_timeout->setInterval(500);
    // connect(m_timeout, &QTimer::timeout, this, [this] {
    //     qDebug() << "timeout error";
    //     // m_queue.clear();
    //     m_timeout->stop();
    // });

    m_connectTimer->setInterval(1000);
    connect(m_connectTimer, &QTimer::timeout, this, &WebSocketInterface::connectDevice);
    m_connectTimer->start();
    /*
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [this] {
        if (m_serialPort->isOpen()) {
            uint16_t stick_1_x = 621;
            uint16_t stick_1_y = 722;
            uint16_t stick_2_x = 823;
            uint16_t stick_2_y = 924;
            uint8_t data[8] {
                uint8_t(stick_1_x >> 8), uint8_t(stick_1_x),
                uint8_t(stick_1_y >> 8), uint8_t(stick_1_y),
                uint8_t(stick_2_x >> 8), uint8_t(stick_2_x),
                uint8_t(stick_2_y >> 8), uint8_t(stick_2_y),
            };
            sendPacket(MSG_ID_SET_MOVE_PARAMS, data, 8);
        }
    });
    timer->start();
    */
}

void WebSocketInterface::sendMessage(const QByteArray &msg) {
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->sendBinaryMessage(msg);
    } else {
        // qDebug() << "The message was not sent. WebSocket is not connected";
    }
}

void WebSocketInterface::setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y) {
    // qDebug() << stick_1_x << stick_1_y << stick_2_x << stick_2_y;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << MSG_ID_SET_MOVE_PARAMS << stick_1_x << stick_1_y << stick_2_x << stick_2_y;
    sendMessage(data);
}

void WebSocketInterface::setMotorPwm(uint16_t left_motor_pwm, uint16_t right_motor_pwm)
{
    QByteArray data;
    QByteArrayWriter writer(data);
    writer << MSG_ID_SET_MOTOR_PWM << left_motor_pwm << right_motor_pwm;
    sendMessage(data);
}

void WebSocketInterface::setMotorSpeeds(float lin_vel, float ang_vel)
{
    QByteArray data;
    QByteArrayWriter writer(data);
    writer << MSG_ID_SET_MOTOR_SPEEDS << lin_vel << ang_vel;
    sendMessage(data);
}

void WebSocketInterface::setWaypoint(float x, float y, float yaw, bool relative) {
    QByteArray data;
    QByteArrayWriter writer(data);
    writer << MSG_ID_SET_WAYPOINT << x << y << yaw << relative;
    sendMessage(data);
}

void WebSocketInterface::addWaypoint(float x, float y, float yaw, bool relative) {
    QByteArray data;
    QByteArrayWriter writer(data);
    writer << MSG_ID_ADD_WAYPOINT << x << y << yaw << relative;
    sendMessage(data);
}

void WebSocketInterface::clearWaypoints() {
    QByteArray data;
    QByteArrayWriter writer(data);
    writer << MSG_ID_CLEAR_WAYPOINTS;
    sendMessage(data);
}

void WebSocketInterface::startBuildingMap() {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << MSG_ID_START_BUILDING_MAP;
    sendMessage(data);
}

void WebSocketInterface::finishBuildingMap() {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << MSG_ID_FINISH_BUILDING_MAP;
    sendMessage(data);
}

void WebSocketInterface::setMowingEnabled(bool enable) {
    if (enable) {
        sendMessage(QByteArray(1, MSG_ID_MOWING_ENABLE));
    } else {
        sendMessage(QByteArray(1, MSG_ID_MOWING_DISABLE));
    }
}

void WebSocketInterface::connectDevice() {
    if (m_webSocket->state() == QAbstractSocket::UnconnectedState) {
        qDebug() << "try connect";
        // m_webSocket->open(QUrl(QString("ws://%1:%2/api").arg("localhost").arg(8081)));
        m_webSocket->open(QUrl(QString("ws://%1:%2/api").arg("192.168.0.0").arg(8081)));
        // m_webSocket->open(QUrl(QString("ws://%1:%2/api").arg("192.168.1.65").arg(8081)));
        // m_webSocket->open(QUrl(QString("ws://%1:%2/api").arg("192.168.31.21").arg(8081)));
    }
}

void WebSocketInterface::disconnectDevice() {
    if (m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        m_webSocket->close();
        m_connectTimer->start();
    }
}

void WebSocketInterface::messageHandler(const QByteArray &msg) {
    uint8_t function_code;
    QByteArrayReader stream(msg);
    stream >> function_code;
    switch(function_code) {
    case MSG_ID_SET_MOVE_PARAMS: {
        // uint16_t stick_1_x = uint16_t(data[0]) << 8 | data[1];
        // uint16_t stick_1_y = uint16_t(data[2]) << 8 | data[3];
        // uint16_t stick_2_x = uint16_t(data[4]) << 8 | data[5];
        // uint16_t stick_2_y = uint16_t(data[6]) << 8 | data[7];
        // qDebug() << "=========================";
        // qDebug() << "stick_1_x: " << stick_1_x;
        // qDebug() << "stick_1_y: " << stick_1_y;
        // qDebug() << "stick_2_x: " << stick_2_x;
        // qDebug() << "stick_2_y: " << stick_2_y;
        break;
    }
    case MSG_ID_YAW_CHANGED: {
        float yaw;
        stream >> yaw;
        qDebug() << "yaw:" << yaw;
        emit yawChanged(yaw);
        break;
    }
    case MSG_ID_LOCAL_POSITION_CHANGED: {
        float x, y, yaw;
        stream >> x >> y >> yaw;
        emit localPositionChanged(x, y, yaw);
        break;
    }
    case MSG_ID_GLOBAL_POSITION_CHANGED: {
        float latitude, longitude;
        uint16_t altitude;
        bool valid;
        stream >> latitude >> longitude >> altitude >> valid;
        qDebug() << "latitude:" << latitude << "longitude:" << longitude << "altitude:" << altitude << "valid:" << valid;
        emit globalPositionChanged(latitude, longitude, altitude);
        break;
    }
    case MSG_ID_SCAN_RECEIVED: {
        size_t size;
        stream >> size;
        QList<QPointF> pointCloud(size);
        for (size_t i = 0; i < size; ++i) {
            float x, y;
            stream >> x >> y;
            pointCloud[i] = QPointF(x, y);
        }
        emit scanReceived(pointCloud);
        break;
    }
    case MSG_ID_VELOCITIES_CHANGED: {
        float vx, vy, v_yaw;
        stream >> vx >> vy >> v_yaw;
        emit velocitiesChanged(vx, vy, v_yaw);
        break;
    }
    // case MSG_ID_MAP_CELL_POSITION_CHANGED: {
    //     QPoint cellPos;
    //     stream >> cellPos;
    //     emit mapCellPositionChanged(cellPos);
    //     break;
    // }
    case MSG_ID_MAP_UPDATED: {
        uint32_t width, height;
        float resolution;
        double ox, oy;
        QByteArray cells_data;
        stream >> width >> height >> resolution >> ox >> oy >> cells_data;
        emit mapChanged(width, height, resolution, ox, oy, cells_data);
        break;
    }
    case MSG_ID_WAYPOINTS_UPDATED: {
        uint32_t size;
        stream >> size;
        QList<QPointF> waypoints(size);
        for (uint32_t i = 0; i < size; ++i) {
            float x, y;
            stream >> x >> y;
            waypoints[i] = {x, y};
        }
        // qDebug() << waypoints;
        emit waypointsUpdated(waypoints);
        break;
    }
    case MSG_ID_ENC_CHANGED: {
        float x, y, yaw;
        stream >> x >> y >> yaw;
        emit encPositionChanged(x, y, yaw);
        break;
    }
    default:
        qDebug() << "неизвестный код функции:" << function_code;
        break;
    }
}
