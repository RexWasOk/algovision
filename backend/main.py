"""
=============================================================
 AlgoVision — main.py
=============================================================

FastAPI backend tying together the C++ engine and Python ML
layer. Same pattern as Logsense.

Endpoints:
  GET  /                    — health check
  GET  /race                — race all 8 algorithms on synthetic data
  POST /predict              — predict winner for a given array
  POST /sort/{algorithm}     — run one algorithm with step capture
  GET  /algorithms           — list available algorithms
=============================================================
"""

from fastapi import FastAPI, HTTPException, Query, Body
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from pathlib import Path
import bridge
from ml import predict as ml_predict

app = FastAPI(title="AlgoVision API", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

FRONTEND_PATH = Path(__file__).parent.parent / "frontend"
app.mount("/app", StaticFiles(directory=str(FRONTEND_PATH),
          html=True), name="frontend")


@app.get("/")
def root():
    return {"status": "ok", "service": "algovision"}


@app.get("/algorithms")
def list_algorithms():
    return {
        "sorting": ["bubble", "selection", "insertion", "merge",
                    "quick", "heap", "shell", "counting"],
        "graph": ["bfs", "dfs", "dijkstra", "astar"]
    }


@app.get("/race")
def race_endpoint(
    size: int = Query(1000, le=20000),
    sorted_ratio: float = Query(0.0, ge=0, le=1),
    duplicate_ratio: float = Query(0.1, ge=0, le=1),
    variance: float = Query(1.0, ge=0.01, le=50)
):
    """Race all 8 sorting algorithms on a synthetic array."""
    try:
        results = bridge.race(size, sorted_ratio, duplicate_ratio, variance)

        # get ML prediction for the same parameters
        prediction = ml_predict.predict(
            size, sorted_ratio, duplicate_ratio, variance
        )

        # sort results by actual time, fastest first
        results.sort(key=lambda r: r["time_ms"])

        return {
            "input": {
                "size": size,
                "sorted_ratio": sorted_ratio,
                "duplicate_ratio": duplicate_ratio,
                "variance": variance
            },
            "results": results,
            "actual_winner": results[0]["algorithm"],
            "ml_prediction": prediction
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/predict")
def predict_endpoint(arr: list[int] = Body(...)):
    """Given a real array, extract features and predict the winner."""
    try:
        features = bridge.extract_features(arr)
        prediction = ml_predict.predict(
            features["size"],
            features["sorted_ratio"],
            features["duplicate_ratio"],
            features["variance"]
        )
        return {"features": features, "prediction": prediction}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/sort/{algorithm}")
def sort_endpoint(algorithm: str, arr: list[int] = Body(...)):
    """Run one algorithm with full step capture for frontend animation."""
    try:
        return bridge.sort_with_steps(algorithm, arr)
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
    
@app.post("/graph/{algorithm}")
def graph_endpoint(algorithm: str, data: dict = Body(...)):
    try:
        import subprocess
        from pathlib import Path

        BASE_DIR = Path(__file__).parent.parent
        ENGINE   = BASE_DIR / "engine" / "algovision"

        edges     = data.get("edges", [])
        positions = data.get("positions", {})
        start     = data["start"]
        end       = data["end"]

        # write a simple text format C++ can parse trivially:
        # line 1: start end num_edges num_positions
        # next lines: from to weight
        # next lines: node_id x y
        tmp = BASE_DIR / "data" / "graph_tmp.txt"
        tmp.parent.mkdir(exist_ok=True)

        with open(tmp, "w", newline='\n') as f:
            f.write(f"{start} {end} {len(edges)} {len(positions)}\n")
            for e in edges:
                f.write(f"{e['from']} {e['to']} {e['weight']}\n")
            for node_id, pos in positions.items():
                f.write(f"{node_id} {pos[0]} {pos[1]}\n")

        result = subprocess.run(
            [str(ENGINE), "graph", algorithm, str(tmp)],
            capture_output=True, text=True, timeout=30
        )
        if result.returncode != 0:
            raise RuntimeError(result.stderr.strip())

        import json
        return json.loads(result.stdout)
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))