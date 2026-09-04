#include "hobot_slasher_protocol.h"
//#include "wit_hwt.h"
#include <SoftwareSerial.h>
#include <Adafruit_BNO08x.h>
#include <iarduino_GPS_NMEA.h>
#include <iarduino_GPS_ATGM336.h>

const float angle_kp = 10.f;
const uint16_t allowance = 20;

HardwareSerial &debug_serial = Serial;
HardwareSerial &radio_serial = Serial1;
HardwareSerial &slave_serial = Serial2;
//HardwareSerial &compass_serial = Serial3;
//HardwareSerial &gps_serial = Serial3;
SoftwareSerial gps_serial(2, 3);

HobotSlasherProtocol hsp;

//WitHWT wit_hwt;
Adafruit_BNO08x bno08x;
sh2_SensorValue_t sensorValue;
float yaw = 0;
bool dir = 0;
int16_t start_yaw = 0;
int16_t target_yaw = 0;
uint32_t dir_timer = millis();

iarduino_GPS_NMEA gps;
iarduino_GPS_ATGM336 settings_gps;

uint16_t stick_2_x = 500;

enum AUTOPILOT_STATE {
  AUTOPILOT_DISABLED,
  AUTOPILOT_MOVE_FORWARD,
  AUTOPILOT_MOVE_LEFT,
  AUTOPILOT_MOVE_RIGHT,
};
AUTOPILOT_STATE autopilot_state = AUTOPILOT_MOVE_FORWARD;

void autopilotEnableCallback(bool enabled) {
  if (enabled) {
    autopilot_state = AUTOPILOT_MOVE_FORWARD;
    //start_yaw = wit_hwt.yaw();
    start_yaw = yaw;
    target_yaw = start_yaw;
    dir_timer = millis();
    Serial.println("autopilot enabled");
  } else {
    autopilot_state = AUTOPILOT_DISABLED;
    Serial.println("autopilot disabled");
  }
}

void setup() {
  debug_serial.begin(9600);
  radio_serial.begin(9600);
  slave_serial.begin(9600);

  hsp.setSerial(&slave_serial);  // установить порт для работы с протоколом
  hsp.setAutopilotEnableCallback(autopilotEnableCallback);

  // wit_hwt.setup();
  // wit_hwt.poll();
  // delay(100);
  // wit_hwt.poll();
  // start_yaw = wit_hwt.yaw();
  // target_yaw = start_yaw;
  // dir_timer = millis();

  // compass
  Wire.setWireTimeout(250000);
  while (!bno08x.begin_I2C(0x4B)) {
    debug_serial.println("Failed to find BNO08x chip");
    delay(1000);
    debug_serial.println("BNO08x reconnect...");
  }
  debug_serial.println("BNO08x connected");
  bno08x.enableReport(SH2_ARVR_STABILIZED_RV, 5000);
  delay(200);
  bno08x.getSensorEvent(&sensorValue);
  delay(200);
  //bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED, 10000);
  if (bno08x.getSensorEvent(&sensorValue)) {
    switch (sensorValue.sensorId) {
      case SH2_ARVR_STABILIZED_RV:
        quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV);
        break;
    }
  }
  autopilotEnableCallback(true);

  // gps
  gps.begin(gps_serial);
  settings_gps.begin(gps_serial);
  settings_gps.baudrate(9600);
  settings_gps.system(GPS_GP);
  settings_gps.composition(NMEA_GGA, NMEA_RMC);
  settings_gps.model(GPS_PORTABLE);
  settings_gps.updaterate(2);
}

void loop() {
  //----------------bno--------------------
  if (bno08x.wasReset()) {
    bno08x.enableReport(SH2_ARVR_STABILIZED_RV, 5000);
    debug_serial.println("was reset");
  }
  if (bno08x.getSensorEvent(&sensorValue)) {
    compass_timeout = millis();
    switch (sensorValue.sensorId) {
      case SH2_ARVR_STABILIZED_RV:
        quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV);
        break;
      case SH2_GYROSCOPE_CALIBRATED:
        // sensorValue.un.gyroscope.z;
        break;
    }
  }
  static uint32_t compass_timeout = millis();
  if (compass_timeout && millis() - compass_timeout >= 3000) {
    compass_timeout = 0;
    autopilot_state = AUTOPILOT_DISABLED;
    debug_serial.println("timeout");
  }
  //---------------------------------------
  hsp.poll();
  static uint32_t tick_timer = millis();
  if (millis() - tick_timer >= 50) {
    tick_timer = millis();

    //wit_hwt.poll();

    if (autopilot_state == AUTOPILOT_MOVE_FORWARD) {
      //stick_2_x = 500 + float(deltaAngle(wit_hwt.yaw(), target_yaw)) * angle_kp;
      stick_2_x = 500 + float(deltaAngle(yaw, target_yaw)) * angle_kp;
      if (stick_2_x < 0) stick_2_x = 0;
      if (stick_2_x > 1000) stick_2_x = 1000;

      setMoveParams(500, 250, stick_2_x, 500);

      if (millis() - dir_timer >= 10000) {
        setMoveParams(500, 0, 500, 500);
        delay(1000);
        dir = !dir;
        if (dir) {
          autopilot_state = AUTOPILOT_MOVE_LEFT;
          target_yaw = normalizeAngle(start_yaw + 180);
        } else {
          target_yaw = start_yaw;
          autopilot_state = AUTOPILOT_MOVE_RIGHT;
        }
      }
    } else if (autopilot_state == AUTOPILOT_MOVE_LEFT) {
      //if (abs(deltaAngle(wit_hwt.yaw(), target_yaw)) <= allowance) {
      if (abs(deltaAngle(yaw, target_yaw)) <= allowance) {
        autopilot_state = AUTOPILOT_MOVE_FORWARD;
        dir_timer = millis();
      }
      setMoveParams(500, 0, 200, 500);
    } else if (autopilot_state == AUTOPILOT_MOVE_RIGHT) {
      //if (abs(deltaAngle(wit_hwt.yaw(), target_yaw)) <= allowance) {
      if (abs(deltaAngle(yaw, target_yaw)) <= allowance) {
        autopilot_state = AUTOPILOT_MOVE_FORWARD;
        dir_timer = millis();
      }
      setMoveParams(500, 0, 800, 500);
    }
  }
  static uint32_t print_timer = millis();
  if (millis() - print_timer >= 200) {
    print_timer = millis();

    // Serial.println(millis());
    debug_serial.print("yaw: ");
    //Serial.print(wit_hwt.yaw());
    debug_serial.print(yaw);
    debug_serial.print("; target yaw: ");
    debug_serial.print(target_yaw);
    debug_serial.print("; stick_2_x: ");
    debug_serial.print(stick_2_x);
    debug_serial.print("; ");
    switch (autopilot_state) {
      case AUTOPILOT_DISABLED:
        debug_serial.print("autopilot disabled");
        break;
      case AUTOPILOT_MOVE_FORWARD:
        debug_serial.print("forward");
        break;
      case AUTOPILOT_MOVE_LEFT:
        debug_serial.print("left");
        break;
      case AUTOPILOT_MOVE_RIGHT:
        debug_serial.print("right");
        break;
    }
    debug_serial.print('\n');
    //-------------------------------------------------
    radio_serial.print("yaw: ");
    radio_serial.print(yaw);
    radio_serial.print("; target yaw: ");
    radio_serial.print(target_yaw);
    radio_serial.print("; stick_2_x: ");
    radio_serial.print(stick_2_x);
    radio_serial.print("; ");
    switch (autopilot_state) {
      case AUTOPILOT_DISABLED:
        radio_serial.print("autopilot disabled");
        break;
      case AUTOPILOT_MOVE_FORWARD:
        radio_serial.print("forward");
        break;
      case AUTOPILOT_MOVE_LEFT:
        radio_serial.print("left");
        break;
      case AUTOPILOT_MOVE_RIGHT:
        radio_serial.print("right");
        break;
    }
    radio_serial.print('\n');
  }
  //------------------gps------------------
  
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

void quaternionToEuler(float qr, float qi, float qj, float qk) {
  float sqr = sq(qr);
  float sqi = sq(qi);
  float sqj = sq(qj);
  float sqk = sq(qk);

  yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr)) * 180.f / M_PI;
  //pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
  //roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));
}

void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector) {
  quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k);
}
