"""Ball-on-beam plant model and a simple closed-loop check."""

from __future__ import annotations

from dataclasses import dataclass
from math import sin, radians

from .control import Pid

G = 9.80665


@dataclass
class BallBeam:
    """Planar ball-on-beam model: x'' = (5/7) g sin(theta) - a_car."""

    x_m: float = 0.0
    v_m_s: float = 0.0
    g: float = G

    def step(self, theta_deg: float, dt: float, accel_car_m_s2: float = 0.0) -> None:
        acceleration = (5.0 / 7.0) * self.g * sin(radians(theta_deg)) - accel_car_m_s2
        self.v_m_s += acceleration * dt
        self.x_m += self.v_m_s * dt


@dataclass
class HoldResult:
    max_error_cm: float
    final_error_cm: float
    settled: bool


def simulate_hold(
    target_cm: float = 0.0,
    start_cm: float = 5.0,
    seconds: float = 5.0,
    dt: float = 0.002,
    gains: tuple[float, float, float] = (2.2, 0.25, 0.35),
) -> HoldResult:
    """Simulate the outer ball loop around the beam model."""
    plant = BallBeam(x_m=start_cm / 100.0)
    pid = Pid(gains[0], gains[1], gains[2], -12.0, 12.0, integral_band=3.0)
    max_error = 0.0
    final_error = 0.0
    settled = False
    steps = int(seconds / dt)
    for index in range(steps):
        measurement_cm = plant.x_m * 100.0
        error = abs(target_cm - measurement_cm)
        max_error = max(max_error, error)
        final_error = error
        theta = pid.update(target_cm, measurement_cm, dt)
        plant.step(theta, dt)
        if index > steps * 0.6 and error < 0.8:
            settled = True
    return HoldResult(max_error, final_error, settled)
