const API_BASE = "http://localhost:8080";

const state = {
    projectType: null,
    difficulty: null,
    styles: new Set(),
};

let currentSteps = [];
let currentStepIndex = 0;

// Simple line-sketch SVGs per category — no external image assets needed
const SKETCHES = {
    Amigurumi: `<svg viewBox="0 0 100 100" fill="none" stroke="#f4c542" stroke-width="2.5" stroke-linecap="round">
    <circle cx="50" cy="42" r="22"/>
    <circle cx="42" cy="38" r="2.5" fill="#f4c542"/>
    <circle cx="58" cy="38" r="2.5" fill="#f4c542"/>
    <path d="M44 48 Q50 53 56 48"/>
    <path d="M30 58 Q25 70 32 78"/>
    <path d="M70 58 Q75 70 68 78"/>
    <path d="M40 62 Q38 75 42 82"/>
    <path d="M60 62 Q62 75 58 82"/>
  </svg>`,
    Clothing: `<svg viewBox="0 0 100 100" fill="none" stroke="#f4c542" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
    <path d="M35 25 L45 20 L55 20 L65 25 L78 35 L68 45 L62 40 L62 80 L38 80 L38 40 L32 45 L22 35 Z"/>
  </svg>`,
    HomeDecor: `<svg viewBox="0 0 100 100" fill="none" stroke="#f4c542" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
    <rect x="20" y="20" width="26" height="26"/>
    <rect x="54" y="20" width="26" height="26"/>
    <rect x="20" y="54" width="26" height="26"/>
    <rect x="54" y="54" width="26" height="26"/>
  </svg>`,
    Accessories: `<svg viewBox="0 0 100 100" fill="none" stroke="#f4c542" stroke-width="2.5" stroke-linecap="round">
    <path d="M25 20 Q20 50 25 80"/>
    <path d="M35 20 Q30 50 35 80"/>
    <path d="M65 20 Q70 50 65 80"/>
    <path d="M75 20 Q80 50 75 80"/>
  </svg>`,
    Other: `<svg viewBox="0 0 100 100" fill="none" stroke="#f4c542" stroke-width="2.5" stroke-linecap="round">
    <circle cx="50" cy="50" r="28"/>
    <path d="M50 22 Q65 50 50 78 Q35 50 50 22"/>
  </svg>`,
};

// --- Gallery: load and render all patterns on page load ---

async function loadGallery() {
    const galleryEl = document.getElementById("gallery");
    try {
        const res = await fetch(`${API_BASE}/patterns`);
        const patterns = await res.json();

        if (patterns.length === 0) {
            galleryEl.innerHTML = `<p class="error">No patterns in the catalog yet.</p>`;
            return;
        }

        galleryEl.innerHTML = patterns.map(renderGalleryCard).join("");

        document.querySelectorAll(".gallery-card").forEach((card) => {
            card.addEventListener("click", () => {
                const pattern = patterns.find((p) => p.id === parseInt(card.dataset.id));
                openStepModal(pattern);
            });
        });
    } catch (err) {
        galleryEl.innerHTML = `<p class="error">Could not load catalog. Is the server running?</p>`;
    }
}

function renderGalleryCard(p) {
    return `
    <div class="gallery-card" data-id="${p.id}">
      <h4>${p.name}</h4>
      <div class="gmeta">${p.difficulty} · ${p.estimatedTimeHours}h</div>
      <div class="gtags">${p.tags.join(" · ")}</div>
    </div>
  `;
}

loadGallery();

// --- Preference form ---

function setupSingleSelect(containerId, stateKey) {
    const container = document.getElementById(containerId);
    container.addEventListener("click", (e) => {
        if (e.target.tagName !== "BUTTON") return;
        [...container.children].forEach((b) => b.classList.remove("selected"));
        e.target.classList.add("selected");
        state[stateKey] = e.target.dataset.value;
    });
}

function setupMultiSelect(containerId, stateSet) {
    const container = document.getElementById(containerId);
    container.addEventListener("click", (e) => {
        if (e.target.tagName !== "BUTTON") return;
        const val = e.target.dataset.value;
        if (stateSet.has(val)) {
            stateSet.delete(val);
            e.target.classList.remove("selected");
        } else {
            stateSet.add(val);
            e.target.classList.add("selected");
        }
    });
}

setupSingleSelect("projectType", "projectType");
setupSingleSelect("difficulty", "difficulty");
setupMultiSelect("styles", state.styles);

document.getElementById("findBtn").addEventListener("click", async () => {
    const resultsEl = document.getElementById("results");
    resultsEl.innerHTML = "";

    if (!state.projectType || !state.difficulty) {
        resultsEl.innerHTML = `<p class="error">Please select a project type and difficulty.</p>`;
        return;
    }

    const payload = {
        desiredDifficulty: state.difficulty,
        desiredType: state.projectType,
        desiredStyles: [...state.styles],
        maxTimeHours: parseFloat(document.getElementById("maxTime").value) || 10,
        weights: { style: 2.0, time: 1.0 },
    };

    try {
        const res = await fetch(`${API_BASE}/recommend`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload),
        });
        const data = await res.json();

        if (!res.ok) {
            resultsEl.innerHTML = `<p class="error">${data.error}</p>`;
            return;
        }

        if (data.length === 0) {
            resultsEl.innerHTML = `<p>No matching patterns found. Try loosening your filters.</p>`;
            return;
        }

        resultsEl.innerHTML = data.map(renderCard).join("");
        attachCardHandlers(data, payload);
    } catch (err) {
        resultsEl.innerHTML = `<p class="error">Could not reach the server. Is it running?</p>`;
    }
});

function renderCard(result) {
    const p = result.pattern;
    return `
    <div class="card" data-pattern-id="${p.id}">
      <h3>${p.name}</h3>
      <div class="score">Match score: ${(result.score * 100).toFixed(0)}%</div>
      <div class="meta">Difficulty: ${p.difficulty} · Time: ${p.estimatedTimeHours}h</div>
      <div class="tags">${p.tags.join(" · ")}</div>
      <div class="explanation">${result.explanation}</div>
      <div class="card-actions">
        <button class="steps-btn" data-id="${p.id}">View Instructions</button>
        <button class="variation-btn" data-id="${p.id}">Generate AI Variation</button>
      </div>
      <div class="variation-slot"></div>
    </div>
  `;
}

function attachCardHandlers(results, preferences) {
    document.querySelectorAll(".steps-btn").forEach((btn) => {
        btn.addEventListener("click", () => {
            const pattern = results.find((r) => r.pattern.id === parseInt(btn.dataset.id)).pattern;
            openStepModal(pattern);
        });
    });

    document.querySelectorAll(".variation-btn").forEach((btn) => {
        btn.addEventListener("click", async () => {
            const slot = btn.closest(".card").querySelector(".variation-slot");
            slot.textContent = "Generating...";
            try {
                const res = await fetch(`${API_BASE}/variation`, {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        patternId: parseInt(btn.dataset.id),
                        preferences,
                    }),
                });
                const data = await res.json();
                slot.innerHTML = res.ok
                    ? `<div class="variation">${data.variation}</div>`
                    : `<p class="error">${data.error}</p>`;
            } catch {
                slot.innerHTML = `<p class="error">Request failed.</p>`;
            }
        });
    });
}

// --- Step-through instructions modal ---

function openStepModal(pattern) {
    currentSteps = pattern.instructionSteps;
    currentStepIndex = 0;
    document.getElementById("modalTitle").textContent = pattern.name;
    document.getElementById("stepModal").classList.remove("hidden");
    renderStep();
}

function renderStep() {
    document.getElementById("stepText").textContent = currentSteps[currentStepIndex];
    document.getElementById("stepCounter").textContent =
        `Step ${currentStepIndex + 1} of ${currentSteps.length}`;
    document.getElementById("progressFill").style.width =
        `${((currentStepIndex + 1) / currentSteps.length) * 100}%`;
    document.getElementById("prevStep").disabled = currentStepIndex === 0;
    document.getElementById("nextStep").disabled = currentStepIndex === currentSteps.length - 1;
}

document.getElementById("prevStep").addEventListener("click", () => {
    if (currentStepIndex > 0) {
        currentStepIndex--;
        renderStep();
    }
});

document.getElementById("nextStep").addEventListener("click", () => {
    if (currentStepIndex < currentSteps.length - 1) {
        currentStepIndex++;
        renderStep();
    }
});

document.getElementById("closeModal").addEventListener("click", () => {
    document.getElementById("stepModal").classList.add("hidden");
});

document.getElementById("moreIdeasBtn").addEventListener("click", async () => {
    const ideasEl = document.getElementById("ideasResults");

    if (!state.projectType || !state.difficulty) {
        ideasEl.innerHTML = `<p class="error">Pick a project type and difficulty first.</p>`;
        return;
    }

    ideasEl.innerHTML = `<p>Dreaming up new ideas...</p>`;

    const payload = {
        desiredDifficulty: state.difficulty,
        desiredType: state.projectType,
        desiredStyles: [...state.styles],
        maxTimeHours: parseFloat(document.getElementById("maxTime").value) || 10,
        count: 4,
    };

    try {
        const res = await fetch(`${API_BASE}/ideas`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload),
        });
        const data = await res.json();

        if (!res.ok) {
            ideasEl.innerHTML = `<p class="error">${data.error}</p>`;
            return;
        }

        ideasEl.innerHTML = data.map(
            (idea) => `<div class="idea-card"><h4>${idea.name}</h4><p>${idea.description}</p></div>`
        ).join("");
    } catch {
        ideasEl.innerHTML = `<p class="error">Could not reach the server.</p>`;
    }
});