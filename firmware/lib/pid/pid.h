#pragma once

#include <algorithm>
#include <cmath>

// A small, dependency-free PID with conditional integration and anti-windup.
// The derivative term is computed from the measurement to avoid setpoint kicks.
class Pid {
 public:
  Pid(float kp, float ki, float kd, float out_min, float out_max)
      : kp_(kp), ki_(ki), kd_(kd), out_min_(out_min), out_max_(out_max) {}

  void reset() {
    integral_ = 0.0f;
    previous_measurement_ = 0.0f;
    has_previous_ = false;
  }

  float update(float setpoint, float measurement, float dt_s) {
    if (dt_s <= 0.0f) return 0.0f;

    const float error = setpoint - measurement;
    const float p = kp_ * error;

    // Conditional integration: do not wind up during large transients.
    if (std::fabs(error) < integral_band_) {
      integral_ += ki_ * error * dt_s;
    }

    float d = 0.0f;
    if (has_previous_) {
      d = -kd_ * (measurement - previous_measurement_) / dt_s;
    }
    previous_measurement_ = measurement;
    has_previous_ = true;

    float output = p + integral_ + d;

    // Back-calculation anti-windup.
    if (output > out_max_) {
      integral_ -= (output - out_max_);
      output = out_max_;
    } else if (output < out_min_) {
      integral_ -= (output - out_min_);
      output = out_min_;
    }
    return output;
  }

  void setIntegralBand(float band) { integral_band_ = std::max(0.0f, band); }

 private:
  float kp_;
  float ki_;
  float kd_;
  float out_min_;
  float out_max_;
  float integral_ = 0.0f;
  float previous_measurement_ = 0.0f;
  bool has_previous_ = false;
  float integral_band_ = 3.0f;
};
