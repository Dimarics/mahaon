#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "mahaon/imu.h"
#include "mahaon/navigator.h"
#include "mahaon/packetassembler.h"
#include "mahaon/rosbridge.h"
#include "mahaon/serialport.h"
#include "mahaon/slamlauncher.h"
#include "mahaon/stateestimator.h"
#include "mahaon/webserver.h"
#include <boost/process/v2/process.hpp>
#include <boost/asio/readable_pipe.hpp>

class Controller
{
public:
    enum MSG_ID : uint8_t {
        MSG_ID_COMMAND_ACCEPTED = 1,

        MSG_ID_YAW_CHANGED,
        MSG_ID_LOCAL_POSITION_CHANGED,
        MSG_ID_GLOBAL_POSITION_CHANGED,
        MSG_ID_SCAN_RECEIVED,
        MSG_ID_VELOCITIES_CHANGED,

        MSG_ID_SET_MOVE_PARAMS,
        MSG_ID_SET_MOTOR_PWM,
        MSG_ID_SET_MOTOR_SPEEDS,
        MSG_ID_SET_SPEED,
        MSG_ID_SET_WAYPOINT,
        MSG_ID_ADD_WAYPOINT,
        MSG_ID_SET_WAYPOINTS,
        MSG_ID_CLEAR_WAYPOINTS,
        MSG_ID_WAYPOINTS_UPDATED,

        MSG_ID_AUTOPILOT_ENABLE,
        MSG_ID_AUTOPILOT_DISABLE,
        MSG_ID_START_BUILDING_MAP,
        MSG_ID_FINISH_BUILDING_MAP,
        MSG_ID_MAP_UPDATED,
        MSG_ID_MOWING_ENABLE,
        MSG_ID_MOWING_DISABLE,

        // debug
        MSG_ID_ENC_CHANGED,
        MSG_ID_IMU_CHANGED,
    };
    struct Position {
        double x = 0;
        double y = 0;
        double yaw = 0;
    };
    Controller(boost::asio::io_context &io_context);
    void setTwist(double lin_vel, double ang_vel);
    void setMotorPwm(int16_t left_motor, int16_t right_motor);
    void shutdown();

private:
    Position m_pos;
    boost::asio::io_context m_ioContext;
    ROSbridge m_rosBridge;
    WebServer m_webServer;
    SerialPort m_motionController;
    PacketAssembler m_motionControllerPacketAssembler;
    Navigator m_navigator;
    SlamLauncher m_slamLauncher;
    StateEstimator m_stateEstimator;
    void moveToPoint();
};

#endif // CONTROLLER_H
