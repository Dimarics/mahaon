#include "hobot_slasher_protocol.h"
#include "Servo.h"

bool autopilot_enabled = false;
HobotSlasherProtocol hsp;

float throttle = 90;
float rotate = 90;

Servo left_motor;
Servo right_motor;

uint32_t timeout = 0;

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

  // throttle = 112;

  // для реальной телеги
  // throttle = stick_1_y * 90.f / 1000.f + 90.f;
  // rotate = stick_2_x * 180.f / 1000.f;

  // для макета
  float throttle = stick_1_y * 90.f / 1000.f + 90.f;
  float rotate = stick_1_x * 180.f / 1000.f;
  // для реальной телеги
  float v_raw = (throttle - 90.f) / 90.f;
  float turn_raw = (rotate - 90.f) / 90.f;
  float L = v_raw * (1.f + turn_raw);
  float R = v_raw * (1.f - turn_raw);
  float factor = max(1, max(abs(L), abs(R)));
  L = L / factor;
  R = R / factor;
  left_motor.write((L + 1.f) * 90);
  right_motor.write(180 - (R + 1.f) * 90);

  timeout = millis();
}

void setup() {
  left_motor.attach(31);
  right_motor.attach(30);

  Serial.begin(9600);
  Serial2.begin(9600);
  
  hsp.setSerial(&Serial2); // установить порт для работы с протоколом
  
  // установить функцию, которая будет вызываться при получении команды
  hsp.setMoveParamsCallback(moveParamsCallback);

  pinMode(12, INPUT_PULLUP);
}

void loop() {
  if (timeout && millis() - timeout >= 1000) {
    timeout = 0;
  }

  hsp.poll(); // опрос устройства (вызывать чем чаще - тем лучше)
  // if (digitalRead(12)) {
  //   autopilot_enabled = !autopilot_enabled;
  //   if (autopilot_enabled) {
  //     hsp.sendPacket(HobotSlasherProtocol::MSG_ID_AUTOPILOT_DISABLE);
  //   } else {
  //     hsp.sendPacket(HobotSlasherProtocol::MSG_ID_AUTOPILOT_ENABLE);
  //   }
  //   while(digitalRead(12));
  // }
}