#include <Arduino.h>

#include "config.h"
#include "pid.h"

namespace {

enum class State : uint8_t { BOOT, CALIBRATE, READY, RUN, BRAKE, FAULT };

struct BallObservation {
  float x_cm = 0.0f;
  float confidence = 0.0f;
  uint32_t stamp_ms = 0;
  bool valid = false;
};

State state = State::BOOT;
BallObservation ball;
Pid ball_pid(BALL_KP, BALL_KI, BALL_KD, SERVO_MIN_DEG, SERVO_MAX_DEG);
Pid angle_pid(ANGLE_KP, ANGLE_KI, ANGLE_KD, -1.0f, 1.0f);
Pid line_pid(LINE_KP, LINE_KI, LINE_KD, -1.0f, 1.0f);

volatile int32_t left_pulses = 0;
volatile int32_t right_pulses = 0;
float left_speed_cm_s = 0.0f;
float right_speed_cm_s = 0.0f;
float previous_speed_cm_s = 0.0f;
float feedforward_deg = 0.0f;
float last_ball_cm = 0.0f;
uint32_t last_control_ms = 0;
uint32_t last_display_ms = 0;
uint32_t start_ms = 0;
uint32_t last_line_ms = 0;

void IRAM_ATTR onLeftEncoder() { left_pulses++; }
void IRAM_ATTR onRightEncoder() { right_pulses++; }

void setMotor(uint8_t pwm_pin, uint8_t dir_pin, float command) {
  command = constrain(command, -1.0f, 1.0f);
  const bool forward = command >= 0.0f;
  digitalWrite(dir_pin, forward ? HIGH : LOW);
  const int duty = static_cast<int>(fabsf(command) * 1023.0f);
  ledcWrite(pwm_pin, duty);
}

void setServoAngle(float degrees) {
  degrees = constrain(degrees, SERVO_MIN_DEG, SERVO_MAX_DEG);
  // 50 Hz, 16-bit: 2.5%..12.5% maps to -90..+90 degrees.
  const float pulse = 0.025f + (degrees + 90.0f) / 180.0f * 0.10f;
  const uint32_t duty = static_cast<uint32_t>(pulse * 65535.0f);
  ledcWrite(PIN_SERVO, duty);
}

float readLineError(bool& lost) {
  // Weighted average of the five IR channels. Higher ADC = more reflective.
  static const float weights[5] = {-2.0f, -1.0f, 0.0f, 1.0f, 2.0f};
  float sum = 0.0f;
  float weight_sum = 0.0f;
  for (int i = 0; i < 5; ++i) {
    const int raw = analogRead(PIN_IR[i]);
    const float strength = static_cast<float>(raw) / 4095.0f;
    sum += weights[i] * strength;
    weight_sum += strength;
  }
  lost = weight_sum < 0.15f;
  return lost ? 0.0f : sum / weight_sum;
}

void parseVisionLine() {
  // Vision sends: B,<x_cm>,<confidence>,<stamp_ms>\n
  static char buffer[64];
  static size_t length = 0;
  while (Serial1.available() > 0) {
    const char c = static_cast<char>(Serial1.read());
    if (c == '\n') {
      buffer[length] = '\0';
      float x = 0.0f;
      float confidence = 0.0f;
      unsigned long stamp = 0;
      if (sscanf(buffer, "B,%f,%f,%lu", &x, &confidence, &stamp) == 3) {
        ball.x_cm = constrain(x, -BALL_TARGET_LIMIT_CM, BALL_TARGET_LIMIT_CM);
        ball.confidence = confidence;
        ball.stamp_ms = static_cast<uint32_t>(stamp);
        ball.valid = confidence > 0.3f;
      }
      length = 0;
    } else if (length < sizeof(buffer) - 1) {
      buffer[length++] = c;
    }
  }
}

float readBatteryVoltage() {
  // 1/11 divider; 12-bit ADC, 3.3 V reference.
  const int raw = analogRead(PIN_VBAT);
  return (static_cast<float>(raw) / 4095.0f) * 3.3f * 11.0f;
}

void updateSpeedEstimate(uint32_t now_ms) {
  static uint32_t last_ms = 0;
  static int32_t last_left = 0;
  static int32_t last_right = 0;
  if (last_ms == 0) {
    last_ms = now_ms;
    last_left = left_pulses;
    last_right = right_pulses;
    return;
  }
  const float dt = (now_ms - last_ms) / 1000.0f;
  if (dt <= 0.0f) return;
  const float left_delta = static_cast<float>(left_pulses - last_left);
  const float right_delta = static_cast<float>(right_pulses - last_right);
  // 11 pulses per wheel revolution, 6.5 cm wheel diameter; tune on the car.
  constexpr float CM_PER_PULSE = 3.1415926f * 6.5f / 11.0f;
  left_speed_cm_s = left_delta * CM_PER_PULSE / dt;
  right_speed_cm_s = right_delta * CM_PER_PULSE / dt;
  last_ms = now_ms;
  last_left = left_pulses;
  last_right = right_pulses;
}

void updateDisplay(uint32_t now_ms) {
  if (now_ms - last_display_ms < 200) return;
  last_display_ms = now_ms;
  const char* state_name = "BOOT";
  switch (state) {
    case State::BOOT: state_name = "BOOT"; break;
    case State::CALIBRATE: state_name = "CAL"; break;
    case State::READY: state_name = "READY"; break;
    case State::RUN: state_name = "RUN"; break;
    case State::BRAKE: state_name = "BRAKE"; break;
    case State::FAULT: state_name = "FAULT"; break;
  }
  const uint32_t elapsed = state == State::RUN ? (now_ms - start_ms) : 0;
  Serial.printf("STATE=%s t=%lu ball=%.2f target=%.2f vbat=%.2f ff=%.2f\n",
                state_name, static_cast<unsigned long>(elapsed), ball.x_cm,
                last_ball_cm, readBatteryVoltage(), feedforward_deg);
}

void enterFault(const char* reason) {
  state = State::FAULT;
  Serial.printf("FAULT: %s\n", reason);
  setMotor(PIN_MOTOR_PWM_L, PIN_MOTOR_DIR_L, 0.0f);
  setMotor(PIN_MOTOR_PWM_R, PIN_MOTOR_DIR_R, 0.0f);
  setServoAngle(0.0f);
  digitalWrite(PIN_BUZZER, HIGH);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 21, 22);  // vision UART (RX, TX)

  pinMode(PIN_MOTOR_DIR_L, OUTPUT);
  pinMode(PIN_MOTOR_DIR_R, OUTPUT);
  pinMode(PIN_KEY, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_ENC_L, INPUT_PULLUP);
  pinMode(PIN_ENC_R, INPUT_PULLUP);

  ledcSetup(0, 20000, 10);
  ledcAttachPin(PIN_MOTOR_PWM_L, 0);
  ledcSetup(1, 20000, 10);
  ledcAttachPin(PIN_MOTOR_PWM_R, 1);
  ledcSetup(2, 50, 16);
  ledcAttachPin(PIN_SERVO, 2);

  attachInterrupt(digitalPinToInterrupt(PIN_ENC_L), onLeftEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_R), onRightEncoder, RISING);

  setServoAngle(0.0f);
  ball_pid.reset();
  angle_pid.reset();
  line_pid.reset();
  state = State::CALIBRATE;
  Serial.println("dsh-git H-problem firmware boot");
}

void loop() {
  const uint32_t now_ms = millis();
  parseVisionLine();
  updateSpeedEstimate(now_ms);

  if (now_ms - last_control_ms < CONTROL_PERIOD_MS) {
    updateDisplay(now_ms);
    return;
  }
  const float dt = (now_ms - last_control_ms) / 1000.0f;
  last_control_ms = now_ms;

  const bool vision_fresh = ball.valid && (now_ms - ball.stamp_ms) < VISION_TIMEOUT_MS;
  if (!vision_fresh && state == State::RUN && (now_ms - ball.stamp_ms) > 1000) {
    enterFault("vision timeout");
  }

  if (state == State::CALIBRATE) {
    setServoAngle(0.0f);
    if (readBatteryVoltage() > 6.6f) {
      state = State::READY;
      digitalWrite(PIN_BUZZER, LOW);
    }
  } else if (state == State::READY) {
    if (digitalRead(PIN_KEY) == LOW) {
      delay(30);
      if (digitalRead(PIN_KEY) == LOW) {
        state = State::RUN;
        start_ms = now_ms;
        ball_pid.reset();
        line_pid.reset();
        last_ball_cm = 0.0f;
      }
    }
  } else if (state == State::RUN) {
    bool lost = false;
    const float line_error = readLineError(lost);
    if (lost) {
      if (now_ms - last_line_ms > LINE_TIMEOUT_MS) {
        enterFault("line lost");
      }
    } else {
      last_line_ms = now_ms;
    }

    const float steering = line_pid.update(0.0f, line_error, dt);
    const float base = 0.62f;
    setMotor(PIN_MOTOR_PWM_L, PIN_MOTOR_DIR_L, base + steering);
    setMotor(PIN_MOTOR_PWM_R, PIN_MOTOR_DIR_R, base - steering);

    if (vision_fresh) {
      last_ball_cm = ball.x_cm;
    }
    const float target_deg = ball_pid.update(0.0f, last_ball_cm, dt);

    // Feed-forward from longitudinal acceleration (cm/s^2 -> m/s^2).
    const float avg_speed = 0.5f * (left_speed_cm_s + right_speed_cm_s);
    const float accel = (avg_speed - previous_speed_cm_s) / dt / 100.0f;
    previous_speed_cm_s = avg_speed;
    feedforward_deg = FEEDFORWARD_GAIN * accel * 57.2958f;

    const float command = constrain(target_deg + feedforward_deg, SERVO_MIN_DEG, SERVO_MAX_DEG);
    const float angle_error = command - 0.0f;  // replace with measured angle if available
    const float servo_cmd = angle_pid.update(0.0f, angle_error, dt);
    setServoAngle(command + servo_cmd * 0.0f);  // angle loop placeholder; see docs

    if (digitalRead(PIN_KEY) == LOW) {
      state = State::BRAKE;
    }
  } else if (state == State::BRAKE) {
    setMotor(PIN_MOTOR_PWM_L, PIN_MOTOR_DIR_L, 0.0f);
    setMotor(PIN_MOTOR_PWM_R, PIN_MOTOR_DIR_R, 0.0f);
    setServoAngle(0.0f);
    if (now_ms - start_ms > 300) {
      state = State::READY;
    }
  } else if (state == State::FAULT) {
    setMotor(PIN_MOTOR_PWM_L, PIN_MOTOR_DIR_L, 0.0f);
    setMotor(PIN_MOTOR_PWM_R, PIN_MOTOR_DIR_R, 0.0f);
    setServoAngle(0.0f);
  }

  updateDisplay(now_ms);
}
