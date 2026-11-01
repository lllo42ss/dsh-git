"""Reusable control helpers for the H-problem prototype."""

from __future__ import annotations

from dataclasses import dataclass, field


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


@dataclass
class Pid:
    """Discrete PID with conditional integration and back-calculation."""

    kp: float
    ki: float
    kd: float
    out_min: float
    out_max: float
    integral_band: float = 3.0
    integral: float = 0.0
    previous_measurement: float | None = None
    last_output: float = 0.0

    def reset(self) -> None:
        self.integral = 0.0
        self.previous_measurement = None
        self.last_output = 0.0

    def update(self, setpoint: float, measurement: float, dt: float) -> float:
        if dt <= 0:
            return self.last_output
        error = setpoint - measurement
        proportional = self.kp * error
        if abs(error) < self.integral_band:
            self.integral += self.ki * error * dt
        derivative = 0.0
        if self.previous_measurement is not None:
            derivative = -self.kd * (measurement - self.previous_measurement) / dt
        self.previous_measurement = measurement
        output = proportional + self.integral + derivative
        if output > self.out_max:
            self.integral -= output - self.out_max
            output = self.out_max
        elif output < self.out_min:
            self.integral -= output - self.out_min
            output = self.out_min
        self.last_output = output
        return output


def mix_motors(base: float, steering: float) -> tuple[float, float]:
    """Mix a signed base speed and steering into left/right commands."""
    left = clamp(base + steering, -1.0, 1.0)
    right = clamp(base - steering, -1.0, 1.0)
    return left, right


def weighted_line_error(sensors: list[float], weights: list[float]) -> tuple[float, bool]:
    """Return (normalized error, lost). sensors are 0..1 reflectivity values."""
    if len(sensors) != len(weights):
        raise ValueError("sensors and weights must have the same length")
    total = sum(sensors)
    if total < 0.15:
        return 0.0, True
    return sum(w * s for w, s in zip(weights, sensors)) / total, False
