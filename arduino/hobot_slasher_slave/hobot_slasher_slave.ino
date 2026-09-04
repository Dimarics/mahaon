#include "hobot_slasher_protocol.h"

bool autopilot_enabled = false;
HobotSlasherProtocol hsp;

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

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  
  hsp.setSerial(&Serial2); // установить порт для работы с протоколом
  
  // установить функцию, которая будет вызываться при получении команды
  hsp.setMoveParamsCallback(moveParamsCallback);

  pinMode(12, INPUT_PULLUP);
}

void loop() {
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