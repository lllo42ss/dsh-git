#!/usr/bin/env python3
"""Repository self-check used by CI and by the pre-commit hook."""

from __future__ import annotations

import sys
from pathlib import Path

REQUIRED = [
    "README.md",
    "LICENSE",
    ".gitignore",
    "docs/requirements.md",
    "docs/architecture.md",
    "docs/control-design.md",
    "docs/test-plan.md",
    "firmware/platformio.ini",
    "firmware/src/main.cpp",
    "vision/protocol.py",
    "simulation/ball_beam.py",
    "tests/test_ball_beam.py",
]


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    missing = [path for path in REQUIRED if not (root / path).is_file()]
    if missing:
        print("Missing files:")
        for path in missing:
            print(f"  - {path}")
        return 1
    readme = (root / "README.md").read_text(encoding="utf-8")
    if "车载平衡滚球" not in readme:
        print("README does not describe the H-problem project")
        return 1
    print(f"OK: {len(REQUIRED)} required files present")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
