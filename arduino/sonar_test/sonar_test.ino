#include "hobot_slasher_protocol.h"

// #define TRIG_PIN_4 28
// #define ECHO_PIN_4 29
// #define TRIG_PIN_3 26
// #define ECHO_PIN_3 27
// #define TRIG_PIN_2 24
// #define ECHO_PIN_2 25
// #define TRIG_PIN_1 22
// #define ECHO_PIN_1 23

// #define TRIG_PIN_5 8
// #define ECHO_PIN_5 9
// #define TRIG_PIN_6 6
// #define ECHO_PIN_6 7

struct Sonar {
  Sonar(uint8_t trig, uint8_t echo)
    : trig_pin(trig), echo_pin(echo) {}
  uint8_t trig_pin;
  uint8_t echo_pin;
  float distance = 65535;
};

Sonar sonars[6] = {
  Sonar(6, 7), Sonar(22, 23), Sonar(24, 25),
  Sonar(26, 27), Sonar(28, 29), Sonar(8, 9)
};

void setup() {
  Serial.begin(9600);
  for (Sonar &sonar : sonars) {
    pinMode(sonar.trig_pin, OUTPUT);
    pinMode(sonar.echo_pin, INPUT);
  }
}

void loop() {
  static uint32_t tick_timer = millis();
  if (millis() - tick_timer >= 100) {
    tick_timer = millis();

    int count = 1;
    Serial.println("\n=====================");
    for (Sonar &sonar : sonars) {
      digitalWrite(sonar.trig_pin, HIGH);
      delayMicroseconds(10);
      digitalWrite(sonar.trig_pin, LOW);
      sonar.distance = float(pulseIn(sonar.echo_pin, HIGH, 100000)) / 5.82;
      if (sonar.distance == 0) sonar.distance = 65535;
      Serial.print("sonar ");
      Serial.print(count);
      Serial.print(": ");
      Serial.print(sonar.distance);
      Serial.print("; ");
      ++count;
    }
  }
}
