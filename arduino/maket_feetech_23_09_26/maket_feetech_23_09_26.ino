#include "hobot_slasher_protocol.h"
#include <SCServo.h>
#include <Adafruit_BNO08x.h>

#define M_PI 3.14159265358979323846

const uint8_t dir_pin = 40;
const uint8_t left_motor_id = 3;
const uint8_t right_motor_id = 2;

int16_t left_motor_enc;
int16_t right_motor_enc;

SMS_STS st;
HobotSlasherProtocol hsp;
Adafruit_BNO08x bno08x;
sh2_SensorValue_t sensorValue;
uint32_t timeout = 0;

// функция для обработки значений джойстиков
void moveParamsCallback(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y) {
  float throttle = stick_1_y * 90.f / 1000.f + 90.f;
  float rotate = stick_1_x * 180.f / 1000.f;
  float v_raw = (throttle - 90.f) / 90.f;
  float turn_raw = (rotate - 90.f) / 90.f;
  float L = v_raw * (1.f + turn_raw);
  float R = v_raw * (1.f - turn_raw);
  float factor = max(1, max(abs(L), abs(R)));
  L = L / factor;
  R = R / factor;
  // left_motor.write((L + 1.f) * 90);
  // right_motor.write(180 - (R + 1.f) * 90);

  timeout = millis();
}

void motorSpeedsCallback(int16_t left_motor_val, int16_t right_motor_val) {
  Serial.print(left_motor_val);
  Serial.print(" ");
  Serial.println(right_motor_val);
  st.WriteSpeed(left_motor_id, -left_motor_val);
  st.WriteSpeed(right_motor_id, right_motor_val);
  timeout = millis();
}

void setup() {
  Serial.begin(9600);

  Serial2.begin(1000000);
  st.pSerial = &Serial2;
  st.DirPin = dir_pin;
  pinMode(dir_pin, OUTPUT);
  delay(200);
  // digitalWrite(st.DirPin, LOW);
  st.WheelMode(left_motor_id);
  st.WheelMode(right_motor_id);
  st.WriteSpeed(left_motor_id, 0);
  st.WriteSpeed(right_motor_id, 0);
  // st.WriteSpeed(left_motor_id, 4000);
  // st.WriteSpeed(right_motor_id, -4000);
  left_motor_enc = st.ReadPos(left_motor_id);
  right_motor_enc = st.ReadPos(right_motor_id);

  Serial3.begin(9600);
  hsp.setSerial(&Serial3);
  hsp.setMoveParamsCallback(moveParamsCallback);
  hsp.setMotorSpeedsCallback(motorSpeedsCallback);

  // bno
  // Wire.setWireTimeout(250000);
  // while (!bno08x.begin_I2C(0x4B)) {
  //   Serial.println("Failed to find BNO08x chip");
  //   delay(1000);
  //   Serial.println("BNO08x reconnect...");
  // }
  // Serial.println("BNO08x connected");
  // bno08x.enableReport(SH2_ARVR_STABILIZED_RV, 100000);
  // bno08x.enableReport(SH2_ROTATION_VECTOR, 100000);
  // bno08x.enableReport(SH2_LINEAR_ACCELERATION, 100000);
  // delay(200);
  // bno08x.getSensorEvent(&sensorValue);
  // delay(200);
  // //bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED, 10000);
  // if (bno08x.getSensorEvent(&sensorValue)) {
  //   switch (sensorValue.sensorId) {
  //     case SH2_ARVR_STABILIZED_RV:
  //       quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV);
  //       break;
  //   }
  // }

  // double wheelBase = 0.16;
  // double lin_vel = 0.20;
  // double ang_vel = 0.0;

  // double left_speed = lin_vel - ang_vel * wheelBase / 2.0;
  // double right_speed = lin_vel + ang_vel * wheelBase / 2.0;

  // // Преобразуем скорости колес в PWM
  // constexpr double to_rpm = 60.0 / M_PI / 0.066;
  // constexpr double to_pwm = 4096.0 / 60.0;
  // double left_motor_pwm = left_speed * to_rpm * to_pwm;
  // double right_motor_pwm = right_speed * to_rpm * to_pwm;
  
  // st.WriteSpeed(left_motor_id, 4096);
  // st.WriteSpeed(right_motor_id, 4096);
}

void loop() {
  // if (Serial3.available()) {
  //   Serial.println(Serial3.read());
  // }
  // delay(200);

  hsp.poll();
  if (timeout && millis() - timeout >= 1000) {
    timeout = 0;
  }
  static uint32_t encoder_timer = millis();
  if (millis() - encoder_timer >= 30) {
    encoder_timer = millis();
    int16_t new_left_enc = st.ReadPos(left_motor_id);
    int16_t new_right_enc = st.ReadPos(right_motor_id);

    float dl = -normEnc(left_motor_enc, new_left_enc) / 4096.0 * M_PI * 0.066;
    float dr = normEnc(right_motor_enc, new_right_enc) / 4096.0 * M_PI * 0.066;
    hsp.sendEnc(dl, dr);
    
    // Serial.println(dl / 0.03);

    left_motor_enc = new_left_enc;
    right_motor_enc = new_right_enc;
  }

  //----------------bno--------------------
  // static uint32_t compass_timeout = millis();
  // if (bno08x.wasReset()) {
  //   bno08x.enableReport(SH2_ARVR_STABILIZED_RV, 50000);
  //   bno08x.enableReport(SH2_ROTATION_VECTOR, 100000);
  //   bno08x.enableReport(SH2_LINEAR_ACCELERATION, 100000);
  //   Serial.println("was reset");
  // }
  // if (bno08x.getSensorEvent(&sensorValue)) {
  //   compass_timeout = millis();
  //   switch (sensorValue.sensorId) {
  //     case SH2_ARVR_STABILIZED_RV:
  //       quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV);
  //       break;
  //     case SH2_GYROSCOPE_CALIBRATED:
  //       // sensorValue.un.gyroscope.z;
  //       break;
  //     case SH2_ROTATION_VECTOR:
  //       Serial.println("SH2_ROTATION_VECTOR");
  //       // hsp.sendImu(sensorValue.un.rotationVector.real,
  //       //             sensorValue.un.rotationVector.i,
  //       //             sensorValue.un.rotationVector.j,
  //       //             sensorValue.un.rotationVector.k,
  //       //             sensorValue.un.linearAcceleration.x,
  //       //             sensorValue.un.linearAcceleration.y,
  //       //             sensorValue.un.linearAcceleration.z);
  //       break;
  //     case SH2_LINEAR_ACCELERATION:
  //       Serial.println("SH2_LINEAR_ACCELERATION");
  //       hsp.sendImu(sensorValue.un.rotationVector.real,
  //                   sensorValue.un.rotationVector.i,
  //                   sensorValue.un.rotationVector.j,
  //                   sensorValue.un.rotationVector.k,
  //                   sensorValue.un.linearAcceleration.x,
  //                   sensorValue.un.linearAcceleration.y,
  //                   sensorValue.un.linearAcceleration.z);
  //       break;
  //   }
  // }
  // if (compass_timeout && millis() - compass_timeout >= 3000) {
  //   compass_timeout = 0;
  //   Serial.println("bno timeout");
  // }
  //---------------------------------------
}

int16_t rpm(float value) {
  return value * 4096.f / 60.f;
}

int16_t normEnc(int16_t from, int16_t to) {
  int16_t denc = to - from;
  if (denc > 2048) denc -= 4096;
  if (denc < -2048) denc += 4096;
  return denc;
}

void quaternionToEuler(float qr, float qi, float qj, float qk) {
  float sqr = sq(qr);
  float sqi = sq(qi);
  float sqj = sq(qj);
  float sqk = sq(qk);

  float yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr)) * 180.f / M_PI;
  //pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
  //roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));
  // Serial.println(yaw);
}

void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector) {
  quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k);
}
