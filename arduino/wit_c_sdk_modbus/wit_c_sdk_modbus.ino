#include "wit_hwt.h"

WitHWT hwt;

void setup() {
  Serial.begin(115200);
  hwt.setup();
}

void loop() {
  hwt.poll();
  delay(50);
  Serial.print("yaw: ");
  Serial.println(hwt.yaw(), 3);
  /*
  static uint32_t timer = millis();
  if (millis() - timer >= 1000) {
    timer = millis();
  }
  */
  /*
  Serial.println("set angle 0");
  //while (!checkAngle(0));
  Serial.println("move");
  delay(10000);
  Serial.println("set angle 90");
  //while (!checkAngle(90));
  Serial.println("move");
  delay(2000);
  Serial.println("set angle 180");
  //while (!checkAngle(180));
  Serial.println("move");
  delay(10000);
  Serial.println("set angle 90");
  //while (!checkAngle(90));
  Serial.println("move");
  delay(2000);
  */
}
