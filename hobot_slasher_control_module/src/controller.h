#ifndef CONTROLLER_H
#define CONTROLLER_H

#pragma once

#include "lidar.h"
#include "mapper.h"
#include "navigator.h"
#include <QObject>
#include <QSerialPort>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QPointF>
#include <mrpt/nav/reactive/CRobot2NavInterface.h>

class Controller : public QObject, public mrpt::nav::CRobot2NavInterface
{
    Q_OBJECT
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
    Controller();
    bool getCurrentPoseAndSpeeds(
        mrpt::math::TPose2D& curPose,
        mrpt::math::TTwist2D& curVelGlobal,
        mrpt::system::TTimeStamp& timestamp,
        mrpt::math::TPose2D& curOdometry,
        std::string& frame_id) override;
    bool changeSpeeds(const mrpt::kinematics::CVehicleVelCmd& vel_cmd) override;
    bool stop(bool isEmergencyStop = true) override;
    bool senseObstacles(mrpt::maps::CSimplePointsMap& obstacles, mrpt::system::TTimeStamp& timestamp) override;
    mrpt::kinematics::CVehicleVelCmd::Ptr getStopCmd() override;
    mrpt::kinematics::CVehicleVelCmd::Ptr getEmergencyStopCmd() override;

private:
    bool m_mowingActive;

    double m_currentX;
    double m_currentY;
    double m_currentYaw;
    double m_currentVelX;
    double m_currentVelY;
    double m_currentVelZ;
    mrpt::system::TTimeStamp m_currentTime;
    mrpt::obs::CObservationPointCloud::Ptr m_currentPointCloud;

    QSerialPort *m_moveController;
    QWebSocketServer *m_webSocketServer;
    Lidar *m_lidar;
    Mapper *m_mapper;
    Navigator m_navigator;

    void sendMessage(const QByteArray &msg, QWebSocket *socket);
    void setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y);
    void setMotorSpeeds(uint8_t left_motor_pwm, uint8_t right_motor_pwm);

public slots:
    void processScan(const mrpt::obs::CObservation2DRangeScan::Ptr &obs);
    // void processScan(const mrpt::obs::CObservationPointCloud::Ptr &obs);

signals:
    void scanPointsChanged(const QList<QPointF>&);
};

#endif // CONTROLLER_H
