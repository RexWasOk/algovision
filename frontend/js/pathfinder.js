/*
 * =============================================================
 *  AlgoVision — pathfinder.js
 * =============================================================
 *  Interactive grid-based pathfinding visualizer.
 *  User can click to place/remove walls, drag start/end nodes.
 *  Calls the backend /graph endpoint to get visit order and
 *  path, then animates node-by-node exploration followed by
 *  the final path highlight.
 * 
 * Uses fixed canvas pixel dimensions (1320x450) directly
 * instead of getBoundingClientRect() to avoid sizing issues.
 *
 *  The grid is represented as a 2D array of cell states:
 *    0 = empty, 1 = wall, 2 = start, 3 = end
 *
 *  Graph is built as an adjacency list from the grid,
 *  sent to the C++ backend as edges, and results animated.
 * =============================================================
 */

const gridCanvas  = document.getElementById("gridCanvas");
const gridCtx     = gridCanvas.getContext("2d");

// fixed canvas pixel dimensions — must match HTML attributes
const CANVAS_W = 1320;
const CANVAS_H = 450;
const COLS     = 60;
const ROWS     = 22;
const CW       = CANVAS_W / COLS;   // cell width  = 22px
const CH       = CANVAS_H / ROWS;   // cell height ≈ 20.45px

const CELL = { EMPTY: 0, WALL: 1, START: 2, END: 3 };
const COLORS = {
    0: "#1A1F2B",
    1: "#3A4252",
    2: "#4FE0C0",
    3: "#FF6B6B",
    VISITED: "#1E3A4A",
    PATH:    "#4FE0C0"
};

let grid       = [];
let startNode  = { r: 5,  c: 5  };
let endNode    = { r: 15, c: 54 };
let isDrawing  = false;
let drawMode   = 1;
let dragging   = null;
let animating  = false;

// ── init ──────────────────────────────────────────────────
function initGrid() {
    gridCanvas.width  = CANVAS_W;
    gridCanvas.height = CANVAS_H;
    grid = Array.from({length: ROWS}, () => new Array(COLS).fill(CELL.EMPTY));
    grid[startNode.r][startNode.c] = CELL.START;
    grid[endNode.r][endNode.c]     = CELL.END;
    drawGrid();
}

// ── draw ──────────────────────────────────────────────────
function drawGrid(visitedSet = new Set(), pathSet = new Set()) {
    gridCtx.clearRect(0, 0, CANVAS_W, CANVAS_H);

    for (let r = 0; r < ROWS; r++) {
        for (let c = 0; c < COLS; c++) {
            const id = r * COLS + c;
            let color = COLORS[grid[r][c]];

            if (visitedSet.has(id) && grid[r][c] === CELL.EMPTY)
                color = COLORS.VISITED;
            if (pathSet.has(id) &&
                grid[r][c] !== CELL.START &&
                grid[r][c] !== CELL.END)
                color = COLORS.PATH;

            gridCtx.fillStyle = color;
            gridCtx.fillRect(
                Math.floor(c * CW) + 1,
                Math.floor(r * CH) + 1,
                Math.floor(CW) - 2,
                Math.floor(CH) - 2
            );
        }
    }
}

// ── mouse helpers ─────────────────────────────────────────
function getCellFromEvent(e) {
    const rect = gridCanvas.getBoundingClientRect();
    // scale from CSS pixels to canvas pixels
    const scaleX = CANVAS_W / rect.width;
    const scaleY = CANVAS_H / rect.height;
    const x = (e.clientX - rect.left) * scaleX;
    const y = (e.clientY - rect.top)  * scaleY;
    return {
        r: Math.max(0, Math.min(ROWS-1, Math.floor(y / CH))),
        c: Math.max(0, Math.min(COLS-1, Math.floor(x / CW)))
    };
}

// ── mouse events ──────────────────────────────────────────
gridCanvas.addEventListener("mousedown", e => {
    if (animating) return;
    const { r, c } = getCellFromEvent(e);

    if (r === startNode.r && c === startNode.c) {
        dragging = "start"; return;
    }
    if (r === endNode.r && c === endNode.c) {
        dragging = "end"; return;
    }

    isDrawing = true;
    drawMode  = grid[r][c] === CELL.WALL ? 0 : 1;
    grid[r][c] = drawMode ? CELL.WALL : CELL.EMPTY;
    drawGrid();
});

gridCanvas.addEventListener("mousemove", e => {
    if (animating) return;
    const { r, c } = getCellFromEvent(e);

    if (dragging === "start") {
        grid[startNode.r][startNode.c] = CELL.EMPTY;
        startNode = { r, c };
        grid[r][c] = CELL.START;
        drawGrid(); return;
    }
    if (dragging === "end") {
        grid[endNode.r][endNode.c] = CELL.EMPTY;
        endNode = { r, c };
        grid[r][c] = CELL.END;
        drawGrid(); return;
    }

    if (!isDrawing) return;
    if (grid[r][c] === CELL.START || grid[r][c] === CELL.END) return;
    grid[r][c] = drawMode ? CELL.WALL : CELL.EMPTY;
    drawGrid();
});

gridCanvas.addEventListener("mouseup", () => {
    isDrawing = false;
    dragging  = null;
});

// ── build graph ───────────────────────────────────────────
function buildGraph() {
    const edges     = [];
    const positions = {};

    for (let r = 0; r < ROWS; r++) {
        for (let c = 0; c < COLS; c++) {
            if (grid[r][c] === CELL.WALL) continue;
            const id = r * COLS + c;
            positions[id] = [c, r];

            const neighbors = [[r-1,c],[r+1,c],[r,c-1],[r,c+1]];
            for (const [nr, nc] of neighbors) {
                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                if (grid[nr][nc] === CELL.WALL) continue;
                edges.push({ from: id, to: nr*COLS+nc, weight: 1 });
            }
        }
    }

    return {
        edges,
        positions,
        start: startNode.r * COLS + startNode.c,
        end:   endNode.r   * COLS + endNode.c
    };
}

// ── find path ─────────────────────────────────────────────
document.getElementById("findPathBtn").addEventListener("click", async () => {
    if (animating) return;
    animating = true;

    const algo = document.getElementById("graphAlgoSelect").value;
    const graphData = buildGraph();

    try {
        const res = await fetch(`${API_BASE}/graph/${algo}`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(graphData)
        });
        const data = await res.json();
        animateVisited(data.visit_order, data.path);
    } catch (err) {
        console.error("Graph error:", err);
        animating = false;
    }
});

document.getElementById("clearWallsBtn").addEventListener("click", () => {
    if (animating) return;
    for (let r = 0; r < ROWS; r++)
        for (let c = 0; c < COLS; c++)
            if (grid[r][c] === CELL.WALL) grid[r][c] = CELL.EMPTY;
    drawGrid();
});

// ── animation ─────────────────────────────────────────────
function animateVisited(visitOrder, path) {
    let i = 0;
    const visited = new Set();
    const batchSize = Math.max(1, Math.floor(visitOrder.length / 200));

    function step() {
        if (i >= visitOrder.length) {
            animatePath(path, visited);
            return;
        }
        for (let b = 0; b < batchSize && i < visitOrder.length; b++) {
            visited.add(visitOrder[i++]);
        }
        drawGrid(visited, new Set());
        setTimeout(step, 16);
    }
    step();
}

function animatePath(path, visited) {
    let i = 0;
    const pathSet = new Set();

    function step() {
        if (i >= path.length) {
            animating = false;
            return;
        }
        pathSet.add(path[i++]);
        drawGrid(visited, pathSet);
        setTimeout(step, 25);
    }
    step();
}

// ── start ─────────────────────────────────────────────────
initGrid();