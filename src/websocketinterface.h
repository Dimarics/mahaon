#ifndef WEBSOCKETINTERFACE_H
#define WEBSOCKETINTERFACE_H

#include "qpolygon.h"
#include <QtQmlIntegration>
#include <QWebSocket>
#include <QTimer>

class WebSocketInterface : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum MSG_ID : uint8_t {
        MSG_ID_COMMAND_ACCEPTED = 1,
        MSG_ID_SET_MOVE_PARAMS,
        MSG_ID_SET_MOTOR_SPEEDS,
        MSG_ID_SET_SPEED,
        MSG_ID_YAW_CHANGED,
        MSG_ID_LOCAL_POSITION_CHANGED,
        MSG_ID_GLOBAL_POSITION_CHANGED,
        MSG_ID_SCAN_POINTS_CHANGED,
        MSG_ID_MAP_CELL_POSITION_CHANGED,

        MSG_ID_AUTOPILOT_ENABLE,
        MSG_ID_AUTOPILOT_DISABLE,
        MSG_ID_START_BUILDING_MAP,
        MSG_ID_FINISH_BUILDING_MAP,

        MSG_ID_MOWING_ENABLE,
        MSG_ID_MOWING_DISABLE,
    };
    WebSocketInterface(QObject *parent = nullptr);
    Q_INVOKABLE inline bool areaContainsPoint(const QPointF &pos, const QList<QPointF> &coords) {
        return QPolygonF(coords).containsPoint(pos, Qt::OddEvenFill);
    }

private:
    QWebSocket *m_webSocket;
    QTimer *m_connectTimer;
    QTimer *m_timeout;
    void sendMessage(const QByteArray &msg);

public slots:
    Q_INVOKABLE void connectDevice();
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y);
    Q_INVOKABLE void setMotorSpeeds(uint8_t left_motor_pwm, uint8_t right_motor_pwm);

    Q_INVOKABLE void startBuildingMap();
    Q_INVOKABLE void finishBuildingMap();
    Q_INVOKABLE void setMowingEnabled(bool enable);

private slots:
    void messageHandler(const QByteArray &msg);

signals:
    void requestToSend();
    void messageAccepted();
    void yawChanged(float yaw);
    void localPositionChanged(float x, float y, float yaw);
    void globalPositionChanged(float latitude, float longitude, uint16_t altitude);
    void mapCellPositionChanged(const QPoint &cellPos);
};

#endif // WEBSOCKETINTERFACE_H
