"""
=============================================================
 AlgoVision - bridge.py
=============================================================

Same IPC pattern as Logsense - subprocess calls to the C++
engine, JSON parsed from stdout. All diagnostic output in
the C++ engine goes to stderr so stdout stays clean JSON.
=============================================================
"""

import subprocess
import json
from pathlib import Path

BASE_DIR    = Path(__file__).parent.parent
ENGINE_PATH = BASE_DIR / "engine" / "algovision"


def _run(args: list) -> str:
    cmd = [str(ENGINE_PATH)] + args
    result = subprocess.run(
        cmd, capture_output=True, text=True, timeout=30
    )
    if result.returncode != 0:
        raise RuntimeError(f"Engine error: {result.stderr.strip()}")
    return result.stdout.strip()


def race(size: int, sorted_ratio: float,
         duplicate_ratio: float, variance: float) -> list:
    """Run all 8 sorting algorithms, return their stats."""
    output = _run([
        "race", str(size), str(sorted_ratio),
        str(duplicate_ratio), str(variance)
    ])
    return json.loads(output)


def extract_features(arr: list) -> dict:
    """Extract ML features from a given array."""
    arr_str = ",".join(str(x) for x in arr)
    output = _run(["features", arr_str])
    return json.loads(output)


def sort_with_steps(algorithm: str, arr: list) -> dict:
    """Run one algorithm with full step capture for animation."""
    arr_str = ",".join(str(x) for x in arr)
    output = _run(["sort", algorithm, arr_str])
    return json.loads(output)
