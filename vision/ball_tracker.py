"""OpenCV ball tracker prototype for the beam camera.

This module is intentionally optional at import time: CI runs without OpenCV,
while the real car/laptop can install `opencv-python` and use `BallTracker`.
"""

from __future__ import annotations

import argparse
import time

from .protocol import BallObservation, Calibration, encode_observation

try:
    import cv2  # type: ignore
    import numpy as np  # type: ignore
except ImportError:  # pragma: no cover - exercised only without OpenCV
    cv2 = None
    np = None


class BallTracker:
    """Threshold + contour centroid tracker for a dark ball on a light groove."""

    def __init__(self, calibration: Calibration, min_area: int = 80) -> None:
        self.calibration = calibration
        self.min_area = min_area

    def detect(self, frame) -> BallObservation | None:
        if cv2 is None or np is None:
            raise RuntimeError("opencv-python is required for BallTracker.detect")
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        _, mask = cv2.threshold(blurred, 90, 255, cv2.THRESH_BINARY_INV)
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if not contours:
            return None
        contour = max(contours, key=cv2.contourArea)
        area = cv2.contourArea(contour)
        if area < self.min_area:
            return None
        moments = cv2.moments(contour)
        if moments["m00"] == 0:
            return None
        x_px = moments["m10"] / moments["m00"]
        return BallObservation(
            x_cm=self.calibration.to_cm(x_px),
            confidence=min(1.0, area / (self.min_area * 8.0)),
            stamp_ms=int(time.time() * 1000),
        )


def main() -> int:
    parser = argparse.ArgumentParser(description="Track a ball on the beam camera")
    parser.add_argument("--source", default="0", help="camera index or video path")
    parser.add_argument("--centre-px", type=float, default=320.0)
    parser.add_argument("--cm-per-px", type=float, default=0.02)
    parser.add_argument("--sign", type=float, default=1.0)
    args = parser.parse_args()

    if cv2 is None:
        print("opencv-python is not installed; install it to run the tracker")
        return 2

    capture = cv2.VideoCapture(int(args.source) if args.source.isdigit() else args.source)
    if not capture.isOpened():
        print("cannot open video source")
        return 1
    tracker = BallTracker(Calibration(args.centre_px, args.cm_per_px, args.sign))
    try:
        while True:
            ok, frame = capture.read()
            if not ok:
                break
            observation = tracker.detect(frame)
            if observation is not None:
                print(encode_observation(observation).strip(), flush=True)
            cv2.imshow("dsh-git ball tracker", frame)
            if cv2.waitKey(1) & 0xFF == ord("q"):
                break
    finally:
        capture.release()
        cv2.destroyAllWindows()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
