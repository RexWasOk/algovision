/*
 * =============================================================
 *  AlgoVision — app.js
 * =============================================================
 *  Drives the control panel sliders, calls the FastAPI /race
 *  endpoint, and renders both the ML prediction banner and
 *  the per-algorithm result cards with mini bar-chart canvases
 *  showing relative comparisons/swaps.
 * =============================================================
 */

const API_BASE = "http://localhost:8000";

// algorithm display names
const ALGO_NAMES = {
    bubble_sort: "Bubble Sort",
    selection_sort: "Selection Sort",
    insertion_sort: "Insertion Sort",
    merge_sort: "Merge Sort",
    quick_sort: "Quick Sort",
    heap_sort: "Heap Sort",
    shell_sort: "Shell Sort",
    counting_sort: "Counting Sort"
};

// ── wire up sliders to live value display ──────────────────
const sizeSlider = document.getElementById("sizeSlider");
const sortedSlider = document.getElementById("sortedSlider");
const dupSlider = document.getElementById("dupSlider");
const varSlider = document.getElementById("varSlider");

const sizeVal = document.getElementById("sizeVal");
const sortedVal = document.getElementById("sortedVal");
const dupVal = document.getElementById("dupVal");
const varVal = document.getElementById("varVal");

sizeSlider.addEventListener("input", () => sizeVal.textContent = sizeSlider.value);
sortedSlider.addEventListener("input", () => sortedVal.textContent = (sortedSlider.value/100).toFixed(2));
dupSlider.addEventListener("input", () => dupVal.textContent = (dupSlider.value/100).toFixed(2));
varSlider.addEventListener("input", () => varVal.textContent = parseFloat(varSlider.value).toFixed(1));

// ── race button ──────────────────────────────────────────
document.getElementById("raceBtn").addEventListener("click", runRace);

async function runRace() {
    const btn = document.getElementById("raceBtn");
    btn.disabled = true;
    btn.textContent = "Racing...";

    const params = new URLSearchParams({
        size: sizeSlider.value,
        sorted_ratio: (sortedSlider.value / 100).toFixed(2),
        duplicate_ratio: (dupSlider.value / 100).toFixed(2),
        variance: parseFloat(varSlider.value).toFixed(2)
    });

    try {
        const res = await fetch(`${API_BASE}/race?${params}`);
        const data = await res.json();
        renderPrediction(data.ml_prediction, data.actual_winner);
        renderResults(data.results, data.actual_winner);
    } catch (err) {
        document.getElementById("raceGrid").innerHTML =
            `<div class="loading">Error: could not reach API. Is the backend running?</div>`;
    }

    btn.disabled = false;
    btn.textContent = "Run race";
}

function renderPrediction(prediction, actualWinner) {
    const banner = document.getElementById("predictionBanner");
    const text = document.getElementById("predictionText");
    const bars = document.getElementById("confidenceBars");

    banner.classList.add("active");

    const predictedName = ALGO_NAMES[prediction.predicted_winner] || prediction.predicted_winner;
    const correct = prediction.predicted_winner === actualWinner;

    text.innerHTML = `${predictedName} ${correct ? "✓ correct" : "(actual: " + (ALGO_NAMES[actualWinner] || actualWinner) + ")"}`;
    text.style.color = correct ? "var(--accent)" : "var(--warning)";

    // render confidence bars for top 5 algorithms
    bars.innerHTML = "";
    const entries = Object.entries(prediction.confidence_scores).slice(0, 5);
    for (const [algo, score] of entries) {
        const pct = (score * 100).toFixed(1);
        const row = document.createElement("div");
        row.className = "confidence-row";
        row.innerHTML = `
            <div class="name">${ALGO_NAMES[algo] || algo}</div>
            <div class="bar-track"><div class="bar-fill" style="width:${pct}%"></div></div>
            <div class="pct">${pct}%</div>
        `;
        bars.appendChild(row);
    }
}

function renderResults(results, actualWinner) {
    const grid = document.getElementById("raceGrid");
    grid.innerHTML = "";

    const maxTime = Math.max(...results.map(r => r.time_ms));
    const maxComparisons = Math.max(...results.map(r => r.comparisons));

    for (const r of results) {
        const isWinner = r.algorithm === actualWinner;

        const card = document.createElement("div");
        card.className = "algo-card" + (isWinner ? " winner" : "");

        card.innerHTML = `
            <div class="name">
                ${ALGO_NAMES[r.algorithm] || r.algorithm}
                ${isWinner ? '<span class="crown">FASTEST</span>' : ''}
            </div>
            <canvas></canvas>
            <div class="stats">
                <span>Comparisons: ${r.comparisons.toLocaleString()}</span>
                <span class="time">${r.time_ms.toFixed(3)}ms</span>
            </div>
        `;

        grid.appendChild(card);

        // size the canvas to match its actual rendered pixel size
        // this keeps text and bars crisp instead of CSS-stretched
        const canvas = card.querySelector("canvas");
        const rect = canvas.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        canvas.width = rect.width * dpr;
        canvas.height = rect.height * dpr;

        drawBar(canvas, dpr, r.time_ms, maxTime, r.comparisons, maxComparisons);
    }
}

function drawBar(canvas, dpr, time, maxTime, comparisons, maxComparisons) {
    const ctx = canvas.getContext("2d");
    ctx.scale(dpr, dpr);

    const w = canvas.width / dpr, h = canvas.height / dpr;
    ctx.clearRect(0, 0, w, h);

    // labels
    ctx.fillStyle = "#8B92A3";
    ctx.font = "10px monospace";
    ctx.textBaseline = "alphabetic";
    ctx.fillText("time", 10, 14);
    ctx.fillText("comparisons", 10, 48);

    // time bar
    const timeRatio = maxTime > 0 ? time / maxTime : 0;
    const timeWidth = Math.max(4, timeRatio * (w - 20));
    ctx.fillStyle = "#4FE0C0";
    ctx.fillRect(10, 19, timeWidth, 14);

    // comparisons bar
    const compRatio = maxComparisons > 0 ? comparisons / maxComparisons : 0;
    const compWidth = Math.max(4, compRatio * (w - 20));
    ctx.fillStyle = "#8B92A3";
    ctx.fillRect(10, 53, compWidth, 14);
}