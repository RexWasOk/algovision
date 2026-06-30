"""
=============================================================
 AlgoVision — train_model.py
=============================================================

ML Training - Predicting the Fastest Sorting Algorithm
---------------------------------------------------------
We generate thousands of synthetic arrays with varying
characteristics (sorted_ratio, duplicate_ratio, variance,
size), run all 8 sorting algorithms on each via the C++
engine, and record which algorithm was fastest.

This becomes our training data: 4 input features -> label
(the winning algorithm name).

We train a Random Forest classifier; an ensemble of
decision trees that vote on the prediction. It works well
here because the relationship between array characteristics
and the best algorithm is non-linear and rule-based (e.g.
"if nearly sorted, insertion sort wins" is exactly the kind
of rule a decision tree learns naturally).

The trained model is saved to disk using joblib so the
FastAPI backend can load it instantly without retraining.
=============================================================
"""

import subprocess
import json
import random
import joblib
import pandas as pd
from pathlib import Path
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report

BASE_DIR    = Path(__file__).parent.parent.parent
ENGINE_PATH = BASE_DIR / "engine" / "algovision"
MODEL_PATH  = Path(__file__).parent / "model.joblib"


def run_race(size, sorted_ratio, dup_ratio, variance):
    """Run all 8 algorithms on a synthetic array via C++ engine."""
    cmd = [
        str(ENGINE_PATH), "race",
        str(size), str(sorted_ratio), str(dup_ratio), str(variance)
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    return json.loads(result.stdout)


def generate_training_data(num_samples=2000):
    """
    Generate training data by running races with random
    feature combinations and recording the winning algorithm.
    """
    rows = []

    for i in range(num_samples):
        # random feature combination — widened ranges so every
        # algorithm gets a realistic chance to win in some scenario
        size            = random.choice([50, 100, 500, 1000, 2000, 5000, 10000])
        sorted_ratio    = round(random.uniform(0, 1), 2)
        duplicate_ratio = round(random.uniform(0, 0.9), 2)
        # variance now scales up to 50 -> range up to 50,000
        # this makes counting sort expensive on high-variance data
        # so quick/merge/heap sort win more often there
        variance        = round(random.uniform(0.01, 50), 2)

        try:
            results = run_race(size, sorted_ratio, duplicate_ratio, variance)
        except Exception as e:
            print(f"Skipping sample {i}: {e}")
            continue

        winner = min(results, key=lambda r: r["time_ms"])

        rows.append({
            "size": size,
            "sorted_ratio": sorted_ratio,
            "duplicate_ratio": duplicate_ratio,
            "variance": variance,
            "winner": winner["algorithm"]
        })

        if (i+1) % 200 == 0:
            print(f"Generated {i+1}/{num_samples} samples")

    return pd.DataFrame(rows)


def train():
    print("Generating training data via C++ engine races...")
    df = generate_training_data(num_samples=2000)

    print(f"\nTotal samples: {len(df)}")
    print(f"Winner distribution:\n{df['winner'].value_counts()}")

    # features and labels
    X = df[["size", "sorted_ratio", "duplicate_ratio", "variance"]]
    y = df["winner"]

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42
    )

    # train Random Forest — 100 trees, each voting on the prediction
    model = RandomForestClassifier(n_estimators=100, random_state=42)
    model.fit(X_train, y_train)

    # evaluate
    y_pred = model.predict(X_test)
    accuracy = accuracy_score(y_test, y_pred)

    print(f"\nTest accuracy: {accuracy*100:.2f}%")
    print("\nClassification report:")
    print(classification_report(y_test, y_pred))

    # save model and training data
    joblib.dump(model, MODEL_PATH)
    df.to_csv(Path(__file__).parent / "training_data.csv", index=False)

    print(f"\nModel saved to {MODEL_PATH}")
    return model, accuracy


if __name__ == "__main__":
    train()