#include "hdp.h"

class HobotSlasherProtocol : public HDP {
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
    MSG_ID_SET_WAYPOINTS,

    MSG_ID_AUTOPILOT_ENABLE,
    MSG_ID_AUTOPILOT_DISABLE,
    MSG_ID_START_BUILDING_MAP,
    MSG_ID_FINISH_BUILDING_MAP,
    MSG_ID_MAP_UPDATED,
    MSG_ID_MOWING_ENABLE,
    MSG_ID_MOWING_DISABLE,

    MSG_ID_ENC_CHANGED,
    MSG_ID_IMU_CHANGED,
  };
  typedef void (*MoveParamsCallback)(uint16_t, uint16_t, uint16_t, uint16_t);
  typedef void (*MotorSpeedsCallback)(int16_t, int16_t);
  typedef void (*AutopilotEnable)(bool);

  HobotSlasherProtocol() {}

  void setMoveParamsCallback(MoveParamsCallback move_params_callback) {
    m_moveParamsCallback = move_params_callback;
  }

  void setMotorSpeedsCallback(MotorSpeedsCallback motor_speeds_callback) {
    m_motorSpeedsCallback = motor_speeds_callback;
  }

  void setAutopilotEnableCallback(AutopilotEnable autopilot_enable_callback) {
    m_autopilotEnableCallback = autopilot_enable_callback;
  }

  void poll() override {
    HDP::poll();
  }

  void sendEnc(float x, float y) {
    sendPacket(MSG_ID_ENC_CHANGED, x, y);
  }

  void sendImu(float real, float i, float j, float k, float ax, float ay, float az) {
    sendPacket(MSG_ID_IMU_CHANGED, real, i, j, k, ax, ay, az);
  }

  void messageHandler(uint8_t msg_id, uint8_t* data) override {
    switch (msg_id) {
      case MSG_ID_SET_MOVE_PARAMS:
        {
          //m_timeout = millis();
          uint16_t stick_1_x = uint16_t(data[0]) << 8 | data[1];
          uint16_t stick_1_y = uint16_t(data[2]) << 8 | data[3];
          uint16_t stick_2_x = uint16_t(data[4]) << 8 | data[5];
          uint16_t stick_2_y = uint16_t(data[6]) << 8 | data[7];
          m_moveParamsCallback(stick_1_x, stick_1_y, stick_2_x, stick_2_y);
          break;
        }
      case MSG_ID_SET_MOTOR_SPEEDS:
        {
          int16_t left_motor_val = int16_t(data[1]) << 8 | data[0];
          int16_t right_motor_val = int16_t(data[3]) << 8 | data[2];
          m_motorSpeedsCallback(left_motor_val, right_motor_val);
          break;
        }
    }
  }

private:
  uint32_t m_timeout = millis();
  MoveParamsCallback m_moveParamsCallback;
  MotorSpeedsCallback m_motorSpeedsCallback;
  AutopilotEnable m_autopilotEnableCallback;
};