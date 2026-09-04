#include "hobot_slasher_protocol.h"

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
  
  hsp.setSerial(&Serial); // установить порт для работы с протоколом
  
  // установить функцию, которая будет вызываться при получении команды
  hsp.setMoveParamsCallback(moveParamsCallback);
}

void loop() {
  hsp.poll(); // опрос устройства (вызывать чем чаще - тем лучше)

  // отправка пакета
  // uint16_t stick_1_x = 621;
  // uint16_t stick_1_y = 722;
  // uint16_t stick_2_x = 823;
  // uint16_t stick_2_y = 924;
  // uint8_t data[8] {
  //   uint8_t(stick_1_x >> 8),
  //   uint8_t(stick_1_x),
  //   uint8_t(stick_1_y >> 8),
  //   uint8_t(stick_1_y),
  //   uint8_t(stick_2_x >> 8),
  //   uint8_t(stick_2_x),
  //   uint8_t(stick_2_y >> 8),
  //   uint8_t(stick_2_y),
  // };
  // hsp.sendPacket(HobotSlasherProtocol::MSG_ID_SET_MOVE_PARAMS, data, 8);
  // delay(1000);
}