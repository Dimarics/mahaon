#include "hobot_slasher_protocol.h"

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

const float angle_kp = 16.f;
const uint16_t allowance = 20;

int speed = 300;
int rotate_speed = 300;
int forward_rotate_speed = 100;

HardwareSerial &debug_serial = Serial;
HardwareSerial &radio_serial = Serial2;
HardwareSerial &slave_serial = Serial1;
HardwareSerial &gps_serial = Serial3;

HobotSlasherProtocol slave;

float yaw = 0;
bool dir = 0;
int16_t start_yaw = 0;
int16_t target_yaw = 0;
uint32_t dir_timer = millis();

uint16_t stick_2_x = 500;

enum AUTOPILOT_STATE {
  AUTOPILOT_DISABLED,
  AUTOPILOT_MOVE_FORWARD,
  AUTOPILOT_MOVE_LEFT,
  AUTOPILOT_MOVE_RIGHT,
};
AUTOPILOT_STATE autopilot_state = AUTOPILOT_DISABLED;

enum ORIENTATION {
  ORIENTATION_LEFT,
  ORIENTATION_RIGHT,
};
ORIENTATION orientation = ORIENTATION_LEFT;

void autopilotEnableCallback(bool enabled) {
  if (enabled) {
    autopilot_state = AUTOPILOT_MOVE_FORWARD;
    //start_yaw = wit_hwt.yaw();
    start_yaw = yaw;
    target_yaw = start_yaw;
    dir_timer = millis();
    Serial.println("autopilot enabled");
    for (Sonar &sonar : sonars) {
      digitalWrite(sonar.trig_pin, HIGH);
      delayMicroseconds(10);
      digitalWrite(sonar.trig_pin, LOW);
      sonar.distance = float(pulseIn(sonar.echo_pin, HIGH, 100000)) / 5.82;
      if (sonar.distance == 0) sonar.distance = 65535;
    }
    Serial.print("orientation: ");
    if (sonars[0].distance <= sonars[5].distance) {
      orientation = ORIENTATION_LEFT;
      Serial.println("LEFT");
    } else {
      orientation = ORIENTATION_RIGHT;
      Serial.println("RIGHT");
    }
  } else {
    autopilot_state = AUTOPILOT_DISABLED;
    Serial.println("autopilot disabled");
  }
}

void setup() {
  debug_serial.begin(9600);

  slave_serial.begin(9600);
  slave.setSerial(&slave_serial);
  slave.setAutopilotEnableCallback(autopilotEnableCallback);

  // autopilotEnableCallback(true);

  // Сонары
  for (Sonar &sonar : sonars) {
    pinMode(sonar.trig_pin, OUTPUT);
    pinMode(sonar.echo_pin, INPUT);
  }
}

void loop() {
  slave.poll();
  static uint32_t tick_timer = millis();
  if (millis() - tick_timer >= 50) {
    tick_timer = millis();

    for (Sonar &sonar : sonars) {
      digitalWrite(sonar.trig_pin, HIGH);
      delayMicroseconds(10);
      digitalWrite(sonar.trig_pin, LOW);
      sonar.distance = float(pulseIn(sonar.echo_pin, HIGH, 100000)) / 5.82;
      if (sonar.distance == 0) sonar.distance = 65535;
    }

    if (autopilot_state == AUTOPILOT_MOVE_FORWARD) {
      if (orientation == ORIENTATION_LEFT) {
        Serial.print("3: ");
        Serial.print(sonars[3].distance);
        Serial.print(", 4: ");
        Serial.println(sonars[4].distance);
        /*if (sonars[0].distance > 1000) {
          stick_2_x = 500 - rotate_speed;
          setMoveParams(500, speed, stick_2_x, 500);
        } else */if (sonars[3].distance < 1000 && sonars[4].distance > 1000) {
          stick_2_x = 500;
          setMoveParams(500, speed, stick_2_x, 500);
        } else if (sonars[3].distance > 1000) {
          stick_2_x = 500 - forward_rotate_speed;
          setMoveParams(500, speed, stick_2_x, 500);
        } else if (sonars[4].distance < 1000) {
          stick_2_x = 500 + forward_rotate_speed;
          setMoveParams(500, speed, stick_2_x, 500);
        }
      } else {
        Serial.print("1: ");
        Serial.print(sonars[1].distance);
        Serial.print(", 2: ");
        Serial.println(sonars[2].distance);
        /*if (sonars[5].distance > 1000) {
          stick_2_x = 500 + rotate_speed;
          setMoveParams(500, speed, stick_2_x, 500);
        } else */if (sonars[2].distance < 1000 && sonars[1].distance > 1000) {
          stick_2_x = 500;
          setMoveParams(500, speed, stick_2_x, 500);
        } else if (sonars[2].distance > 1000) {
          stick_2_x = 500 + forward_rotate_speed;
          setMoveParams(500, speed, stick_2_x, 500);
        } else if (sonars[1].distance < 1000) {
          stick_2_x = 500 - forward_rotate_speed;
          setMoveParams(500, speed, stick_2_x, 500);
        }
      }
      delay(500);
      // stick_2_x = 500 + float(deltaAngle(yaw, target_yaw)) * angle_kp;
      // if (stick_2_x < 0) stick_2_x = 0;
      // if (stick_2_x > 1000) stick_2_x = 1000;

      // setMoveParams(500, 250, stick_2_x, 500);

      // //if ((dir && gps.longitude <= min_longitude) || (!dir && gps.longitude >= max_longitude)) {
      // if (millis() - dir_timer >= 10000) {
      //   setMoveParams(500, 0, 500, 500);
      //   delay(1000);
      //   dir = !dir;
      //   if (dir) {
      //     autopilot_state = AUTOPILOT_MOVE_LEFT;
      //     target_yaw = normalizeAngle(start_yaw + 180);
      //   } else {
      //     target_yaw = start_yaw;
      //     autopilot_state = AUTOPILOT_MOVE_RIGHT;
      //   }
      // }
    } else if (autopilot_state == AUTOPILOT_MOVE_LEFT) {
      // if (abs(deltaAngle(yaw, target_yaw)) <= allowance) {
      //   autopilot_state = AUTOPILOT_MOVE_FORWARD;
      //   dir_timer = millis();
      // }
      if (sonars[0].distance < 1000) {
        autopilot_state = AUTOPILOT_MOVE_FORWARD;
      }
      stick_2_x = 500 - rotate_speed;
      setMoveParams(500, 0, stick_2_x, 500);
    } else if (autopilot_state == AUTOPILOT_MOVE_RIGHT) {
      // if (abs(deltaAngle(yaw, target_yaw)) <= allowance) {
      //   autopilot_state = AUTOPILOT_MOVE_FORWARD;
      //   dir_timer = millis();
      // }
      // setMoveParams(500, 0, 800, 500);
    }
  }
  static uint32_t print_timer = millis();
  if (millis() - print_timer >= 500) {
    print_timer = millis();
    debug_serial.print("stick_2_x: ");
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
  }
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
  slave.sendPacket(HobotSlasherProtocol::MSG_ID_SET_MOVE_PARAMS, data, 8);
}
