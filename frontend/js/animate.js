/*
 * =============================================================
 *  AlgoVision — animate.js
 * =============================================================
 *  Fetches a sorting algorithm's full step-by-step snapshots
 *  from the backend and animates them as bars on a canvas,
 *  one frame per snapshot, using requestAnimationFrame for
 *  smooth 60fps playback.
 * =============================================================
 */

const animCanvas = document.getElementById("animCanvas");
const animCtx = animCanvas.getContext("2d");

function setupCanvasResolution(canvas, ctx) {
    const rect = canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    canvas.width = rect.width * dpr;
    canvas.height = rect.height * dpr;
    ctx.scale(dpr, dpr);
    return { w: rect.width, h: rect.height };
}

let animFrameId = null;

document.getElementById("animBtn").addEventListener("click", runAnimation);

const animSizeSlider = document.getElementById("animSizeSlider");
const animSizeVal = document.getElementById("animSizeVal");
animSizeSlider.addEventListener("input", () => animSizeVal.textContent = animSizeSlider.value);

async function runAnimation() {
    if (animFrameId) cancelAnimationFrame(animFrameId);

    const btn = document.getElementById("animBtn");
    btn.disabled = true;
    btn.textContent = "Loading...";

    const algo = document.getElementById("animAlgoSelect").value;
    const size = parseInt(animSizeSlider.value);

    // generate a random array client-side for the animation
    const arr = Array.from({length: size}, () => Math.floor(Math.random() * 300) + 10);

    try {
        const res = await fetch(`${API_BASE}/sort/${algo}`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(arr)
        });
        const data = await res.json();

        btn.textContent = "Animate";
        btn.disabled = false;

        playSnapshots(data.snapshots, arr);
    } catch (err) {
        btn.textContent = "Animate";
        btn.disabled = false;
        console.error(err);
    }
}

function playSnapshots(snapshots, initialArr) {
    if (!snapshots || snapshots.length === 0) {
        drawBars(initialArr, -1, -1);
        return;
    }

    let frame = 0;
    // throttle: if there are too many snapshots, skip some for smoothness
    const maxFrames = 600;
    const step = Math.max(1, Math.floor(snapshots.length / maxFrames));

    function tick() {
        if (frame >= snapshots.length) {
            drawBars(snapshots[snapshots.length - 1], -1, -1, true);
            return;
        }
        drawBars(snapshots[frame]);
        frame += step;
        animFrameId = requestAnimationFrame(tick);
    }

    tick();
}

function drawBars(arr, highlightA = -1, highlightB = -1, done = false) {
    const { w, h } = setupCanvasResolution(animCanvas, animCtx);
    animCtx.clearRect(0, 0, w, h);

    const maxVal = Math.max(...arr, 1);
    const barWidth = w / arr.length;
    const padding = 2;

    for (let i = 0; i < arr.length; i++) {
        const barHeight = (arr[i] / maxVal) * (h - 20);
        const x = i * barWidth;
        const y = h - barHeight;

        if (done) animCtx.fillStyle = "#4FE0C0";
        else if (i === highlightA || i === highlightB) animCtx.fillStyle = "#FFB454";
        else animCtx.fillStyle = "#3A4252";

        animCtx.fillRect(x + padding/2, y, Math.max(1, barWidth - padding), barHeight);
    }
}

// draw an initial empty state
window.addEventListener("DOMContentLoaded", () => {
    const placeholder = Array.from({length: 60}, () => Math.floor(Math.random()*300)+10);
    drawBars(placeholder);
});