#include "hobot_slasher_protocol.h"

#define TRIG_PIN_4 28
#define ECHO_PIN_4 29
#define TRIG_PIN_3 26
#define ECHO_PIN_3 27
#define TRIG_PIN_2 24
#define ECHO_PIN_2 25
#define TRIG_PIN_1 22
#define ECHO_PIN_1 23

const float angle_kp = 16.f;
const uint16_t allowance = 20;

int speed = 300;
int rotate_speed = 300;
int forward_rotate_speed = 100;

HardwareSerial &debug_serial = Serial;
HardwareSerial &radio_serial = Serial1;
HardwareSerial &slave_serial = Serial2;
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

  slave_serial.begin(9600);
  slave.setSerial(&slave_serial);
  slave.setAutopilotEnableCallback(autopilotEnableCallback);

  // autopilotEnableCallback(true);

  // Сонары
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(TRIG_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
  pinMode(TRIG_PIN_4, OUTPUT);
  pinMode(ECHO_PIN_4, INPUT);
}

void loop() {
  slave.poll();
  static uint32_t tick_timer = millis();
  if (millis() - tick_timer >= 50) {
    tick_timer = millis();

    float distance[4];

    digitalWrite(TRIG_PIN_1, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN_1, LOW);
    distance[0] = float(pulseIn(ECHO_PIN_1, HIGH, 100000)) / 5.82;
    if (distance[0] == 0) distance[0] = 65535;


    digitalWrite(TRIG_PIN_2, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN_2, LOW);
    distance[1] = float(pulseIn(ECHO_PIN_2, HIGH, 100000)) / 5.82;
    if (distance[1] == 0) distance[0] = 65535;


    digitalWrite(TRIG_PIN_3, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN_3, LOW);
    distance[2] = float(pulseIn(ECHO_PIN_3, HIGH, 100000)) / 5.82;
    if (distance[2] == 0) distance[0] = 65535;


    digitalWrite(TRIG_PIN_4, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN_4, LOW);
    distance[3] = float(pulseIn(ECHO_PIN_4, HIGH, 100000)) / 5.82;
    if (distance[3] == 0) distance[0] = 65535;

    if (autopilot_state == AUTOPILOT_MOVE_FORWARD) {
      if (distance[0] > 1000) {
        // autopilot_state = AUTOPILOT_MOVE_LEFT;
        stick_2_x = 500 - rotate_speed;
        setMoveParams(500, speed, stick_2_x, 500);
      } else if (distance[1] < 1000 && distance[2] > 1000) {
        stick_2_x = 500;
        setMoveParams(500, speed, stick_2_x, 500);
      } else if (distance[1] > 1000) {
        stick_2_x = 500 - forward_rotate_speed;
        setMoveParams(500, speed, stick_2_x, 500);
      } else if (distance[2] < 1000) {
        stick_2_x = 500 + forward_rotate_speed;
        setMoveParams(500, speed, stick_2_x, 500);
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
      if (distance[0] < 1000) {
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
