"""
=============================================================
 AlgoVision — predict.py
=============================================================

Loads the trained Random Forest model and exposes a clean
predict() function. Given array features, returns the
predicted fastest algorithm plus confidence scores for
every algorithm the model considered.

predict_proba() returns probability scores for each class
(algorithm) instead of just the top prediction — this is
what lets the frontend show "quick_sort: 73% confidence,
heap_sort: 18% confidence" etc.
=============================================================
"""

import joblib
import pandas as pd
from pathlib import Path

MODEL_PATH = Path(__file__).parent / "model.joblib"

_model = None


def _load_model():
    global _model
    if _model is None:
        _model = joblib.load(MODEL_PATH)
    return _model


def predict(size: int, sorted_ratio: float,
            duplicate_ratio: float, variance: float) -> dict:
    """
    Predict the fastest sorting algorithm for given array
    characteristics. Returns the top prediction plus
    confidence scores for all algorithms.
    """
    model = _load_model()

    X = pd.DataFrame([{
        "size": size,
        "sorted_ratio": sorted_ratio,
        "duplicate_ratio": duplicate_ratio,
        "variance": variance
    }])

    prediction = model.predict(X)[0]
    probabilities = model.predict_proba(X)[0]

    # pair each algorithm class with its confidence score
    classes = model.classes_
    confidence_scores = {
        classes[i]: round(float(probabilities[i]), 4)
        for i in range(len(classes))
    }

    # sort by confidence descending
    confidence_scores = dict(
        sorted(confidence_scores.items(),
               key=lambda x: x[1], reverse=True)
    )

    return {
        "predicted_winner": prediction,
        "confidence_scores": confidence_scores
    }