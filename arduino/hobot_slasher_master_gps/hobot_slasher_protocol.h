#ifndef HOBOT_SLASHER_PROTOCOL
#define HOBOT_SLASHER_PROTOCOL

#include "hdp.h"

class HobotSlasherProtocol : public HDP {
public:
  enum MSG_ID : uint8_t {
    MSG_ID_COMMAND_ACCEPTED = 1,
    MSG_ID_SET_MOVE_PARAMS,
    MSG_ID_AUTOPILOT_ENABLE,
    MSG_ID_AUTOPILOT_DISABLE,
    MSG_ID_GLOBAL_POSITION_CHANGED
  };
  typedef void (*MoveParamsCallback)(uint16_t, uint16_t, uint16_t, uint16_t);
  typedef void (*AutopilotEnable)(bool);

  HobotSlasherProtocol() {}

  void setMoveParamsCallback(MoveParamsCallback move_params_callback) {
    m_moveParamsCallback = move_params_callback;
  }

  void setAutopilotEnableCallback(AutopilotEnable autopilot_enable_callback) {
    m_autopilotEnableCallback = autopilot_enable_callback;
  }

  void poll() override {
    HDP::poll();
    /*
    if (millis() - m_timeout >= 1000) {
      m_moveParamsCallback(512, 0, 512, 512);
      Serial.println("control timeout");
    }
    */
  }

  void messageHandler(uint8_t msg_id, uint8_t* data) override {
    switch (msg_id) {
      case MSG_ID_SET_MOVE_PARAMS:
        {
          m_timeout = millis();
          uint16_t stick_1_x = uint16_t(data[0]) << 8 | data[1];
          uint16_t stick_1_y = uint16_t(data[2]) << 8 | data[3];
          uint16_t stick_2_x = uint16_t(data[4]) << 8 | data[5];
          uint16_t stick_2_y = uint16_t(data[6]) << 8 | data[7];
          m_moveParamsCallback(stick_1_x, stick_1_y, stick_2_x, stick_2_y);
        }
        break;
      case MSG_ID_AUTOPILOT_ENABLE:
        m_autopilotEnableCallback(true);
        break;
      case MSG_ID_AUTOPILOT_DISABLE:
        m_autopilotEnableCallback(false);
        break;
    }
  }

  void sendGlobalCoordinates(float latitude, float longitude, uint16_t altitude) {
    uint8_t data[10];
    memcpy(&data[0], &latitude, 4);
    memcpy(&data[4], &longitude, 4);
    memcpy(&data[8], &altitude, 2);
    sendPacket(MSG_ID_GLOBAL_POSITION_CHANGED, data, 10);
  }

private:
  uint32_t m_timeout = millis();
  MoveParamsCallback m_moveParamsCallback;
  AutopilotEnable m_autopilotEnableCallback;
};

#endif // HOBOT_SLASHER_PROTOCOL