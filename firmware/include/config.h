#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pin map (reference; verify against the selected ESP32-S3 board)
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_IR[5] = {1, 2, 3, 4, 5};  // L2, L1, C, R1, R2
constexpr uint8_t PIN_MOTOR_PWM_L = 6;
constexpr uint8_t PIN_MOTOR_DIR_L = 7;
constexpr uint8_t PIN_MOTOR_PWM_R = 8;
constexpr uint8_t PIN_MOTOR_DIR_R = 9;
constexpr uint8_t PIN_ENC_L = 10;
constexpr uint8_t PIN_ENC_R = 11;
constexpr uint8_t PIN_SERVO = 12;
constexpr uint8_t PIN_KEY = 17;
constexpr uint8_t PIN_BUZZER = 18;
constexpr uint8_t PIN_VBAT = 19;

// ---------------------------------------------------------------------------
// Control constants
// ---------------------------------------------------------------------------
constexpr float CONTROL_HZ = 200.0f;
constexpr uint32_t CONTROL_PERIOD_MS = 5;
constexpr float SERVO_MIN_DEG = -25.0f;
constexpr float SERVO_MAX_DEG = 25.0f;
constexpr float BALL_TARGET_LIMIT_CM = 6.0f;
constexpr uint32_t VISION_TIMEOUT_MS = 150;
constexpr uint32_t LINE_TIMEOUT_MS = 200;

// Ball position loop: cm -> degrees
constexpr float BALL_KP = 2.2f;
constexpr float BALL_KI = 0.25f;
constexpr float BALL_KD = 0.35f;

// Beam angle loop: degrees -> PWM
constexpr float ANGLE_KP = 3.0f;
constexpr float ANGLE_KI = 0.05f;
constexpr float ANGLE_KD = 0.08f;

// Line following loop: normalized error -> steering
constexpr float LINE_KP = 45.0f;
constexpr float LINE_KI = 0.0f;
constexpr float LINE_KD = 8.0f;

// Feed-forward: theta_ff = -(7 / (5g)) * a
constexpr float GRAVITY = 9.80665f;
constexpr float FEEDFORWARD_GAIN = -(7.0f / (5.0f * GRAVITY));
