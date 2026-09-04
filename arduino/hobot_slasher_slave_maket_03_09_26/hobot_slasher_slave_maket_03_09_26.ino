#include "hobot_slasher_protocol.h"
#include "Servo.h"

bool autopilot_enabled = false;
HobotSlasherProtocol hsp;

Servo left_motor;
Servo right_motor;

uint32_t timeout = 0;

// функция для обработки значений джойстиков
void moveParamsCallback(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y) {
  // Serial.print("stick_1_x: ");
  // Serial.print(stick_1_x);
  // Serial.print(" stick_1_y: ");
  // Serial.print(stick_1_y);
  // Serial.print(" stick_2_x: ");
  // Serial.print(stick_2_x);
  // Serial.print(" stick_2_y: ");
  // Serial.println(stick_2_y);

  // throttle = 112;

  // для реальной телеги
  // throttle = stick_1_y * 90.f / 1000.f + 90.f;
  // rotate = stick_2_x * 180.f / 1000.f;

  float throttle = stick_1_y * 90.f / 1000.f + 90.f;
  float rotate = stick_1_x * 180.f / 1000.f;
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

void motorSpeedsCallback(uint8_t left_motor_pwm, uint8_t right_motor_pwm) {
  left_motor.write(left_motor_pwm);
  right_motor.write(180 - right_motor_pwm);
  // Serial.print(left_motor_pwm);
  // Serial.print(", ");
  // Serial.println(right_motor_pwm);
  timeout = millis();
}

void setup() {
  left_motor.attach(31);
  right_motor.attach(30);

  Serial.begin(9600);
  Serial2.begin(9600);
  
  hsp.setSerial(&Serial2);
  hsp.setMoveParamsCallback(moveParamsCallback);
  hsp.setMotorSpeedsCallback(motorSpeedsCallback);

  pinMode(12, INPUT_PULLUP);
}

void loop() {
  if (timeout && millis() - timeout >= 1000) {
    timeout = 0;
    left_motor.write(90);
    right_motor.write(90);
  }
  hsp.poll(); // опрос устройства (вызывать чем чаще - тем лучше)
}