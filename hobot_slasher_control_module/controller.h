#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "lidar.h"
#include "mapper.h"
#include <QObject>
#include <QSerialPort>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QPointF>

class Controller : public QObject
{
    Q_OBJECT
public:
    enum MSG_ID : uint8_t {
        MSG_ID_COMMAND_ACCEPTED = 1,
        MSG_ID_SET_MOVE_PARAMS,
        MSG_ID_AUTOPILOT_ENABLE,
        MSG_ID_AUTOPILOT_DISABLE,
        MSG_ID_YAW_CHANGED,
        MSG_ID_LOCAL_POSITION_CHANGED,
        MSG_ID_GLOBAL_POSITION_CHANGED,

        MSG_ID_START_BUILDING_MAP,
        MSG_ID_FINISH_BUILDING_MAP,
        MSG_ID_SCAN_POINTS_CHANGED,
        MSG_ID_MAP_CELL_POSITION_CHANGED,

        MSG_ID_MOWING_ENABLE,
        MSG_ID_MOWING_DISABLE,
    };
    Controller();

private:
    bool m_mowingActive;
    Lidar *m_lidar;
    Mapper *m_mapper;
    QSerialPort *m_moveController;
    QWebSocketServer *m_webSocketServer;
    void sendMessage(const QByteArray &msg, QWebSocket *socket);
    void setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y);

public slots:
    void processScan(const mrpt::obs::CObservation2DRangeScan::Ptr &obs);
    // void processScan(const mrpt::obs::CObservationPointCloud::Ptr &obs);

signals:
    void scanPointsChanged(const QList<QPointF>&);
};

#endif // CONTROLLER_H
