#include "hobot_slasher_protocol.h"
#include "wit_hwt.h"

const float angle_kp = 180.f;
const uint16_t allowance = 20;

HobotSlasherProtocol hsp;
WitHWT wit_hwt;
bool autopilot_enabled = false;
bool dir = 0;
int16_t start_yaw = 0;
int16_t target_yaw = 0;
uint32_t dir_timer = millis();

// функция для обработки значений джойстиков
void moveParamsCallback(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y) {
  Serial.print("stick_1_x: ");
  Serial.print(stick_1_x);
  Serial.print(" stick_1_y: ");
  Serial.print(stick_1_y);
  Serial.print(" stick_2_x: ");
  Serial.print(stick_2_x);
  Serial.print(" stick_2_y: ");
  Serial.println(stick_2_y);
}

void autopilotEnableCallback(bool state) {
  autopilot_enabled = state;
  if (autopilot_enabled) {
    Serial.println("autopilot enable");
    start_yaw = wit_hwt.yaw();
    target_yaw = start_yaw;
    dir_timer = millis();
  } else {
    Serial.println("autopilot disable");
  }
}

void setup() {
  Serial.begin(9600);

  Serial2.begin(9600);
  hsp.setSerial(&Serial2);  // установить порт для работы с протоколом

  // установить функцию, которая будет вызываться при получении команды
  hsp.setMoveParamsCallback(moveParamsCallback);
  hsp.setAutopilotEnableCallback(autopilotEnableCallback);

  wit_hwt.setup();

  wit_hwt.poll();
  delay(100);
  wit_hwt.poll();
  start_yaw = wit_hwt.yaw();
  target_yaw = start_yaw;
  dir_timer = millis();
}

void loop() {
  hsp.poll();  // опрос устройства (вызывать чем чаще - тем лучше)

  if (autopilot_enabled) {
    static uint32_t tick_timer = millis();
    if (millis() - tick_timer >= 50) {
      tick_timer = millis();
      wit_hwt.poll();

      int16_t stick_2_x = 500 + float(deltaAngle(wit_hwt.yaw(), target_yaw)) / angle_kp * 500.f;
      if (stick_2_x < 0) stick_2_x = 0;
      if (stick_2_x > 1000) stick_2_x = 1000;

      // отправка пакета
      setMoveParams(500, 250, stick_2_x, 500);

      Serial.print("yaw: ");
      Serial.print(wit_hwt.yaw());
      Serial.print("; target yaw: ");
      Serial.print(target_yaw);
      Serial.print("; stick_2_x: ");
      Serial.print(stick_2_x);
      Serial.println("; forward");
    }

    if (millis() - dir_timer >= 10000) {
      setMoveParams(500, 0, 500, 500);
      delay(1000);
      dir = !dir;
      if (dir) {
        target_yaw = normalizeAngle(start_yaw + 180);
        while (abs(deltaAngle(wit_hwt.yaw(), target_yaw)) > allowance) {
          hsp.poll();
          wit_hwt.poll();
          setMoveParams(500, 0, 350, 500);
          Serial.print("yaw: ");
          Serial.print(wit_hwt.yaw());
          Serial.print("; target yaw: ");
          Serial.print(target_yaw);
          Serial.print("; stick_2_x: ");
          Serial.print(0);
          Serial.println("; left");
          delay(50);
        }
        dir_timer = millis();
      } else {
        target_yaw = start_yaw;
        while (abs(deltaAngle(wit_hwt.yaw(), target_yaw)) > allowance) {
          hsp.poll();
          wit_hwt.poll();
          setMoveParams(500, 0, 650, 500);
          Serial.print("yaw: ");
          Serial.print(wit_hwt.yaw());
          Serial.print("; target yaw: ");
          Serial.print(target_yaw);
          Serial.print("; stick_2_x: ");
          Serial.print(1000);
          Serial.println("; right");
          delay(50);
        }
        dir_timer = millis();
      }
    }
  }
}

int16_t normalizeAngle(int16_t yaw) {
  if (yaw > 180) {
    return yaw - 360;
  } else if (yaw <= -180) {
    return yaw + 360;
  }
  return yaw;
}

int16_t deltaAngle(int16_t yaw, int16_t target_yaw) {
  int16_t delta = normalizeAngle(yaw - target_yaw);
  return delta;
}

void setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y) {
  // Serial.print("yaw: ");
  // Serial.print(wit_hwt.yaw());
  // Serial.print("; target yaw: ");
  // Serial.print(target_yaw);
  // Serial.print("; delta: ");
  // Serial.print(delta);
  // Serial.print("; stick_2_x: ");
  // Serial.println(stick_2_x);
  uint8_t data[8]{
    uint8_t(stick_1_x >> 8),
    uint8_t(stick_1_x),
    uint8_t(stick_1_y >> 8),
    uint8_t(stick_1_y),
    uint8_t(stick_2_x >> 8),
    uint8_t(stick_2_x),
    uint8_t(stick_2_y >> 8),
    uint8_t(stick_2_y),
  };
  hsp.sendPacket(HobotSlasherProtocol::MSG_ID_SET_MOVE_PARAMS, data, 8);
}