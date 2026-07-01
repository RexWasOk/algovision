# AlgoVision

An interactive algorithm visualizer with an ML engine that predicts the optimal sorting algorithm for any input distribution — built with a C++ benchmarking engine, Python Random Forest model, and a live web dashboard.

## Demo

![Demo](docs/algovision_demo.gif)

---

## What it does

**Algorithm Race** — configure array characteristics (size, sorted ratio, duplicate ratio, variance) via sliders. The ML model predicts which algorithm will win before the race runs. All 8 sorting algorithms race simultaneously — the actual winner validates the prediction live.

**Sort Animation** — watch any sorting algorithm execute step by step. Every swap and comparison is captured by the C++ engine and replayed as a smooth bar chart animation.

**Pathfinding Visualizer** — draw walls on an interactive grid by clicking and dragging. Place start and end nodes anywhere. Pick BFS, DFS, Dijkstra, or A* and watch the algorithm explore the grid node by node, then highlight the final path.

---

## Getting started

### Prerequisites

- g++ with C++17 support
- Python 3.10+
- Node.js (optional — only needed if serving frontend separately)

### Step 1 — Clone the repo

```bash
git clone https://github.com/RexWasOk/algovision.git
cd algovision
```

### Step 2 — Build the C++ engine

```bash
cd engine
g++ -std=c++17 -O2 -o algovision src/main.cpp src/sorting.cpp src/graphs.cpp src/benchmark.cpp
cd ..
```

### Step 3 — Install Python dependencies

```bash
pip install -r requirements.txt
```

### Step 4 — Train the ML model

This generates 2000 benchmark races via the C++ engine and trains a Random Forest classifier. Takes 2-3 minutes.

```bash
cd backend/ml
python train_model.py
cd ../..
```

You should see:
```
Generated 2000/2000 samples
Test accuracy: 93.25%
Model saved to backend/ml/model.joblib
```

### Step 5 — Start the backend

```bash
cd backend
python -m uvicorn main:app --reload --port 8000
```

You should see:
```
INFO: Uvicorn running on http://127.0.0.1:8000
INFO: Application startup complete.
```

### Step 6 — Open the dashboard

Open your browser and go to:

```
http://localhost:8000/app
```

The full dashboard loads with three sections — race, animation, and pathfinding.

---

## Using the dashboard

### Algorithm Race
1. Move the sliders to configure your array
   - **Array Size** — number of elements (100 to 10,000)
   - **Sorted Ratio** — how sorted the array is (0 = random, 1 = fully sorted)
   - **Duplicate Ratio** — fraction of duplicate values
   - **Variance** — spread of values (low = tight range, high = wide range)
2. Click **Run race**
3. The ML prediction banner shows the predicted winner with confidence scores
4. The race grid shows all 8 algorithms sorted fastest to slowest with timing and comparison counts
5. Check if the ML prediction matched the actual winner

### Sort Animation
1. Pick an algorithm from the dropdown
2. Set the number of elements with the slider (10 to 150)
3. Click **Animate**
4. Watch the bars sort in real time — each frame is one swap captured by the C++ engine

### Pathfinding Visualizer
1. **Click** on the grid to place walls
2. **Click again** on a wall to erase it
3. **Drag** the teal node to move the start position
4. **Drag** the red node to move the end position
5. Pick an algorithm (BFS, DFS, Dijkstra, A*)
6. Click **Find path**
7. Watch the algorithm explore (blue cells) then highlight the shortest path (teal cells)
8. Click **Clear walls** to reset the grid

---

## Architecture

```
Frontend (HTML/CSS/JS + Canvas API)
      ↕  REST + HTTP
FastAPI backend (Python, port 8000)
      ↕  subprocess IPC (text file)
C++ engine (algovision binary)
      ↑
Random Forest ML model (joblib)
```

The C++ engine is a standalone binary called via Python subprocess. It reads input from command line arguments and a simple text file (for graph data), and outputs clean JSON to stdout. This keeps the C++ and Python codebases completely independent — no build complexity, no pybind11, no ABI issues.

---

## ML Model

Trained a **Random Forest** (100 decision trees) on 2000 synthetic benchmark races. Each sample is one race across all 8 algorithms on a randomly generated array. The label is whichever algorithm had the lowest execution time.

**Features used for prediction:**

| Feature | Description |
|---|---|
| `size` | Number of elements |
| `sorted_ratio` | Fraction of adjacent pairs already in order |
| `duplicate_ratio` | 1 - (unique elements / total elements) |
| `variance` | Normalized standard deviation of values |

**Test accuracy: 93.25%** across 400 held-out samples.

---

## Algorithms

| Category | Algorithms |
|---|---|
| Sorting | Bubble, Selection, Insertion, Merge, Quick, Heap, Shell, Counting |
| Pathfinding | BFS, DFS, Dijkstra, A* |

---

## Project structure

```
algovision/
├── engine/
│   ├── src/
│   │   ├── main.cpp        — CLI interface, JSON output for Python bridge
│   │   ├── sorting.cpp     — 8 sorting algorithms with step-by-step capture
│   │   ├── graphs.cpp      — BFS, DFS, Dijkstra, A* with path reconstruction
│   │   └── benchmark.cpp   — array generator + feature extractor
│   └── include/
│       ├── sorting.h
│       ├── graphs.h
│       └── benchmark.h
├── backend/
│   ├── main.py             — FastAPI: race, sort, graph, predict endpoints
│   ├── bridge.py           — subprocess IPC bridge to C++ engine
│   └── ml/
│       ├── train_model.py  — generates 2000 training samples, trains RF model
│       ├── predict.py      — loads model, returns prediction + confidence scores
│       └── model.joblib    — saved trained model
├── frontend/
│   ├── index.html          — dashboard layout
│   ├── css/style.css       — dark technical theme
│   └── js/
│       ├── app.js          — race dashboard, ML confidence bars
│       ├── animate.js      — sort animation via requestAnimationFrame
│       └── pathfinder.js   — interactive grid, pathfinding animation
├── data/                   — temp files for C++ graph IPC
├── docs/                   — demo GIFs
└── requirements.txt
```

---

## Design decisions

**Why C++ for the engine?**
Sorting benchmarks need nanosecond-precision timing and no GC pauses. A Python implementation would add noise to the benchmark measurements making fair comparisons impossible. C++ gives deterministic, reproducible timing.

**Why subprocess IPC over Python bindings?**
Subprocess keeps the C++ and Python codebases fully independent. No pybind11, no build complexity, no ABI compatibility issues. The engine outputs JSON to stdout, Python reads and parses it — simple, debuggable, and language-agnostic.

**Why Random Forest for algorithm prediction?**
The relationship between array characteristics and the fastest algorithm is naturally rule-based. "If nearly sorted → insertion sort wins" is a single decision tree split. Random Forest ensembles 100 such trees for robust, noise-resistant predictions across edge cases.

**Why Canvas API over a charting library?**
The sort animation requires rendering hundreds of frames per second with full control over individual bar colors. Canvas gives direct pixel-level control at 60fps — a charting library would be too slow and inflexible for frame-by-frame animation.