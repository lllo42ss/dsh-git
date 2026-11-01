"""Vision-side ball observation and calibration helpers."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Calibration:
    """Linear pixel-to-centimetre calibration around the beam centre."""

    centre_px: float
    cm_per_px: float
    sign: float = 1.0

    def to_cm(self, x_px: float) -> float:
        return (x_px - self.centre_px) * self.cm_per_px * self.sign


@dataclass(frozen=True)
class BallObservation:
    x_cm: float
    confidence: float
    stamp_ms: int
    valid: bool = True


def parse_observation(line: str) -> BallObservation | None:
    """Parse the firmware-facing serial frame: B,<x>,<confidence>,<stamp>."""
    parts = [p.strip() for p in line.strip().split(",")]
    if len(parts) != 4 or parts[0] != "B":
        return None
    try:
        x_cm = float(parts[1])
        confidence = float(parts[2])
        stamp_ms = int(parts[3])
    except ValueError:
        return None
    return BallObservation(x_cm, max(0.0, min(1.0, confidence)), stamp_ms, confidence > 0.3)


def encode_observation(observation: BallObservation) -> str:
    return f"B,{observation.x_cm:.3f},{observation.confidence:.3f},{observation.stamp_ms}\n"
