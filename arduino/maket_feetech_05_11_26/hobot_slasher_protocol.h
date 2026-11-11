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
  typedef void (*MoveParamsCallback)(uint16_t, uint16_t, uint16_t, uint16_t);
  typedef void (*MotorPwmCallback)(int16_t, int16_t);
  typedef void (*AutopilotEnable)(bool);

  HobotSlasherProtocol()
    : m_moveParamsCallback([](uint16_t, uint16_t, uint16_t, uint16_t){})
    , m_motorPwmCallback([](int16_t, int16_t){})
    , m_autopilotEnableCallback([](bool){}) {}

  void setMoveParamsCallback(MoveParamsCallback move_params_callback) {
    m_moveParamsCallback = move_params_callback;
  }

  void setMotorPwmCallback(MotorPwmCallback motor_pwm_callback) {
    m_motorPwmCallback = motor_pwm_callback;
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
          uint16_t stick_1_x = uint16_t(data[1]) << 8 | data[0];
          uint16_t stick_1_y = uint16_t(data[3]) << 8 | data[2];
          uint16_t stick_2_x = uint16_t(data[5]) << 8 | data[4];
          uint16_t stick_2_y = uint16_t(data[7]) << 8 | data[6];
          m_moveParamsCallback(stick_1_x, stick_1_y, stick_2_x, stick_2_y);
          break;
        }
      case MSG_ID_SET_MOTOR_PWM:
        {
          int16_t left_motor_val = int16_t(data[1]) << 8 | data[0];
          int16_t right_motor_val = int16_t(data[3]) << 8 | data[2];
          m_motorPwmCallback(left_motor_val, right_motor_val);
          break;
        }
    }
  }

private:
  uint32_t m_timeout = millis();
  MoveParamsCallback m_moveParamsCallback;
  MotorPwmCallback m_motorPwmCallback;
  AutopilotEnable m_autopilotEnableCallback;
};