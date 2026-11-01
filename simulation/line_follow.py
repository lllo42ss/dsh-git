"""Line-following helpers used by the firmware and tests."""

from __future__ import annotations

from .control import mix_motors, weighted_line_error

WEIGHTS = [-2.0, -1.0, 0.0, 1.0, 2.0]


def steering_command(sensors: list[float], kp: float = 0.9) -> tuple[float, float, bool]:
    """Return (left, right, lost) for a five-channel IR array."""
    error, lost = weighted_line_error(sensors, WEIGHTS)
    if lost:
        return 0.0, 0.0, True
    steering = max(-1.0, min(1.0, kp * error))
    left, right = mix_motors(0.55, steering)
    return left, right, False
