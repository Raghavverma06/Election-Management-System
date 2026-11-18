// ======================================================
// ELECTRA - ELECTION MANAGEMENT SYSTEM
// Frontend JavaScript
// Backend: C++ cpp-httplib server on localhost:8080
// ======================================================

const API_BASE = "http://127.0.0.1:8080";

// ------------------------------------------------------
// API HELPER
// ------------------------------------------------------

async function apiRequest(endpoint, options = {}) {
    const response = await fetch(`${API_BASE}${endpoint}`, {
        ...options,
        headers: {
            "Content-Type": "application/json",
            ...(options.headers || {})
        }
    });

    let data = {};

    try {
        data = await response.json();
    } catch {
        data = {};
    }

    if (!response.ok) {
        throw new Error(
            data.message ||
            data.error ||
            `Request failed with status ${response.status}`
        );
    }

    return data;
}

// ------------------------------------------------------
// GENERAL HELPERS
// ------------------------------------------------------

function getElement(id) {
    return document.getElementById(id);
}

function getValue(id) {
    return getElement(id).value.trim();
}

function showToast(message, type = "success") {
    const toast = getElement("toast");

    if (!toast) return;

    toast.textContent = message;
    toast.className = `toast show ${type}`;

    setTimeout(() => {
        toast.className = "toast";
    }, 3000);
}

function escapeHTML(value) {
    return String(value ?? "")
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

function getArray(data, possibleKeys = []) {
    if (Array.isArray(data)) {
        return data;
    }

    for (const key of possibleKeys) {
        if (Array.isArray(data?.[key])) {
            return data[key];
        }
    }

    return [];
}

// ------------------------------------------------------
// PAGE NAVIGATION
// ------------------------------------------------------

function showPage(pageName) {
    const pages = document.querySelectorAll(".page");
    const navItems = document.querySelectorAll(".nav-item");

    pages.forEach(page => {
        page.classList.remove("active");
    });

    navItems.forEach(item => {
        item.classList.remove("active");
    });

    const selectedPage = getElement(`${pageName}-page`);

    if (selectedPage) {
        selectedPage.classList.add("active");
    }

    const selectedNav = document.querySelector(
        `.nav-item[data-page="${pageName}"]`
    );

    if (selectedNav) {
        selectedNav.classList.add("active");
    }

    const pageTitles = {
        dashboard: "Dashboard Overview",
        candidates: "Candidate Management",
        voters: "Voter Management",
        voting: "Voting",
        results: "Election Results"
    };

    const pageSubtitles = {
        dashboard: "Welcome back, Administrator!",
        candidates: "Register and manage election candidates",
        voters: "Register eligible voters",
        voting: "Record a vote for an eligible voter",
        results: "Candidate-wise vote count and performance"
    };

    getElement("page-title").textContent =
        pageTitles[pageName] || "Election Management";

    getElement("page-subtitle").textContent =
        pageSubtitles[pageName] || "";

    // Load the relevant page data.
    if (pageName === "dashboard") {
        loadDashboard();
    } else if (pageName === "candidates") {
        loadCandidates();
    } else if (pageName === "voters") {
        loadVoters();
    } else if (pageName === "voting") {
        loadVotingCandidates();
    } else if (pageName === "results") {
        loadResults();
    }
}

// ------------------------------------------------------
// DASHBOARD
// ------------------------------------------------------

async function loadDashboard() {
    try {
        const data = await apiRequest("/api/dashboard");

        const candidates = Number(
            data.totalCandidates ?? data.candidates ?? 0
        );

        const voters = Number(
            data.totalVoters ?? data.voters ?? 0
        );

        const votes = Number(
            data.totalVotes ?? data.votesCast ?? data.votes ?? 0
        );

        const turnout = voters > 0
            ? ((votes / voters) * 100).toFixed(1)
            : "0";

        getElement("total-candidates").textContent = candidates;
        getElement("total-voters").textContent = voters;
        getElement("total-votes").textContent = votes;
        getElement("voter-turnout").textContent = `${turnout}%`;

        await loadDashboardResults();
        await loadRecentCandidates();

    } catch (error) {
        console.error("Dashboard error:", error);
        showToast("Could not load dashboard data. Check the backend.", "error");
    }
}

// ------------------------------------------------------
// DASHBOARD RESULTS PREVIEW
// ------------------------------------------------------

async function loadDashboardResults() {
    const container = getElement("dashboard-results");

    try {
        const data = await apiRequest("/api/results");

        const results = getArray(data, [
            "results",
            "candidates",
            "data"
        ]);

        if (results.length === 0) {
            container.innerHTML = `
                <div class="empty-state">
                    No election results available yet.
                </div>
            `;
            return;
        }

        const totalVotes = results.reduce((sum, candidate) => {
            return sum + Number(
                candidate.votes ??
                candidate.totalVotes ??
                0
            );
        }, 0);

        container.innerHTML = results
            .slice()
            .sort((a, b) => {
                return Number(b.votes ?? b.totalVotes ?? 0) -
                       Number(a.votes ?? a.totalVotes ?? 0);
            })
            .slice(0, 5)
            .map(candidate => {
                const name = candidate.name ?? candidate.candidateName ?? "";
                const party = candidate.party ?? candidate.politicalParty ?? "";
                const votes = Number(
                    candidate.votes ?? candidate.totalVotes ?? 0
                );

                const percentage = totalVotes > 0
                    ? ((votes / totalVotes) * 100).toFixed(1)
                    : "0";

                return `
                    <div class="result-item">
                        <div class="result-info">
                            <div class="result-candidate">
                                <div class="candidate-avatar">
                                    ${escapeHTML(name.charAt(0).toUpperCase())}
                                </div>

                                <div>
                                    <h4>${escapeHTML(name)}</h4>
                                    <p>${escapeHTML(party)}</p>
                                </div>
                            </div>

                            <div class="result-votes">
                                <strong>${votes} votes</strong>
                                <span>${percentage}%</span>
                            </div>
                        </div>

                        <div class="progress-bar">
                            <div
                                class="progress-fill"
                                style="width: ${percentage}%"
                            ></div>
                        </div>
                    </div>
                `;
            })
            .join("");

    } catch (error) {
        console.error("Dashboard results error:", error);

        container.innerHTML = `
            <div class="empty-state">
                Unable to load election results.
            </div>
        `;
    }
}

// ------------------------------------------------------
// RECENT CANDIDATES
// ------------------------------------------------------

async function loadRecentCandidates() {
    const table = getElement("recent-candidates");

    try {
        const data = await apiRequest("/api/candidates");

        const candidates = getArray(data, [
            "candidates",
            "data"
        ]);

        if (candidates.length === 0) {
            table.innerHTML = `
                <tr>
                    <td colspan="4" class="empty-state">
                        No candidates registered yet.
                    </td>
                </tr>
            `;
            return;
        }

        table.innerHTML = candidates
            .slice(0, 5)
            .map(candidate => `
                <tr>
                    <td>${escapeHTML(candidate.id ?? candidate.candidateID ?? "")}</td>
                    <td>${escapeHTML(candidate.name ?? candidate.candidateName ?? "")}</td>
                    <td>${escapeHTML(candidate.party ?? candidate.politicalParty ?? "")}</td>
                    <td>${Number(candidate.votes ?? candidate.totalVotes ?? 0)}</td>
                </tr>
            `)
            .join("");

    } catch (error) {
        console.error("Recent candidates error:", error);

        table.innerHTML = `
            <tr>
                <td colspan="4" class="empty-state">
                    Unable to load candidates.
                </td>
            </tr>
        `;
    }
}

// ------------------------------------------------------
// CANDIDATE REGISTRATION
// ------------------------------------------------------

async function registerCandidate(event) {
    event.preventDefault();

    const id = getValue("candidate-id");
    const name = getValue("candidate-name");
    const party = getValue("candidate-party");

    if (!id || !name || !party) {
        showToast("Please fill in all candidate fields.", "error");
        return;
    }

    try {
        await apiRequest("/api/candidates", {
            method: "POST",
            body: JSON.stringify({
                id: Number(id),
                name: name,
                party: party
            })
        });

        showToast("Candidate registered successfully!");

        getElement("candidate-form").reset();

        await loadCandidates();
        await loadDashboard();

    } catch (error) {
        console.error("Candidate registration error:", error);
        showToast(error.message || "Candidate registration failed.", "error");
    }
}

// ------------------------------------------------------
// LOAD CANDIDATES
// ------------------------------------------------------

async function loadCandidates() {
    const table = getElement("candidates-table");

    try {
        const data = await apiRequest("/api/candidates");

        const candidates = getArray(data, [
            "candidates",
            "data"
        ]);

        if (candidates.length === 0) {
            table.innerHTML = `
                <tr>
                    <td colspan="4" class="empty-state">
                        No candidates registered yet.
                    </td>
                </tr>
            `;
            return;
        }

        table.innerHTML = candidates.map(candidate => `
            <tr>
                <td>${escapeHTML(candidate.id ?? candidate.candidateID ?? "")}</td>
                <td>${escapeHTML(candidate.name ?? candidate.candidateName ?? "")}</td>
                <td>${escapeHTML(candidate.party ?? candidate.politicalParty ?? "")}</td>
                <td>${Number(candidate.votes ?? candidate.totalVotes ?? 0)}</td>
            </tr>
        `).join("");

    } catch (error) {
        console.error("Candidates loading error:", error);

        table.innerHTML = `
            <tr>
                <td colspan="4" class="empty-state">
                    Unable to load candidates.
                </td>
            </tr>
        `;
    }
}

// ------------------------------------------------------
// VOTER REGISTRATION
// ------------------------------------------------------

async function registerVoter(event) {
    event.preventDefault();

    const id = getValue("voter-id");
    const name = getValue("voter-name");

    if (!id || !name) {
        showToast("Please fill in all voter fields.", "error");
        return;
    }

    try {
        await apiRequest("/api/voters", {
            method: "POST",
            body: JSON.stringify({
                voterID: id,
                name: name
            })
        });

        showToast("Voter registered successfully!");

        getElement("voter-form").reset();

        await loadVoters();
        await loadDashboard();

    } catch (error) {
        console.error("Voter registration error:", error);
        showToast(error.message || "Voter registration failed.", "error");
    }
}

// ------------------------------------------------------
// LOAD VOTERS
// ------------------------------------------------------

async function loadVoters() {
    const table = getElement("voters-table");

    try {
        const data = await apiRequest("/api/voters");

        const voters = getArray(data, [
            "voters",
            "data"
        ]);

        if (voters.length === 0) {
            table.innerHTML = `
                <tr>
                    <td colspan="3" class="empty-state">
                        No voters registered yet.
                    </td>
                </tr>
            `;
            return;
        }

        table.innerHTML = voters.map(voter => {
            const voterID = voter.voterID ?? voter.id ?? "";
            const name = voter.name ?? voter.voterName ?? "";

            const hasVoted =
                voter.voted ??
                voter.hasVoted ??
                voter.votingStatus ??
                false;

            const voted = hasVoted === true ||
                hasVoted === 1 ||
                String(hasVoted).toLowerCase() === "true" ||
                String(hasVoted).toLowerCase() === "voted";

            return `
                <tr>
                    <td>${escapeHTML(voterID)}</td>
                    <td>${escapeHTML(name)}</td>
                    <td>
                        <span class="status-badge ${voted ? "voted" : "not-voted"}">
                            ${voted ? "Voted" : "Not Voted"}
                        </span>
                    </td>
                </tr>
            `;
        }).join("");

    } catch (error) {
        console.error("Voters loading error:", error);

        table.innerHTML = `
            <tr>
                <td colspan="3" class="empty-state">
                    Unable to load voters.
                </td>
            </tr>
        `;
    }
}

// ------------------------------------------------------
// LOAD CANDIDATES INTO VOTING DROPDOWN
// ------------------------------------------------------

async function loadVotingCandidates() {
    const select = getElement("vote-candidate-id");

    try {
        const data = await apiRequest("/api/candidates");

        const candidates = getArray(data, [
            "candidates",
            "data"
        ]);

        select.innerHTML = `
            <option value="">Select a candidate</option>
        `;

        candidates.forEach(candidate => {
            const id = candidate.id ?? candidate.candidateID;
            const name = candidate.name ?? candidate.candidateName;
            const party = candidate.party ?? candidate.politicalParty ?? "";

            const option = document.createElement("option");

            option.value = id;
            option.textContent = `${name} (${party})`;

            select.appendChild(option);
        });

    } catch (error) {
        console.error("Voting candidates error:", error);
        showToast("Unable to load candidates for voting.", "error");
    }
}

// ------------------------------------------------------
// CAST VOTE
// ------------------------------------------------------

async function castVote(event) {
    event.preventDefault();

    const voterID = getValue("vote-voter-id");
    const candidateID = getValue("vote-candidate-id");

    if (!voterID || !candidateID) {
        showToast("Please enter voter ID and select a candidate.", "error");
        return;
    }

    try {
        await apiRequest("/api/vote", {
            method: "POST",
            body: JSON.stringify({
                voterID: voterID,
                candidateID: Number(candidateID)
            })
        });

        showToast("Vote cast successfully!");

        getElement("vote-form").reset();

        await loadVotingCandidates();
        await loadVoters();
        await loadDashboard();
        await loadResults();

    } catch (error) {
        console.error("Voting error:", error);
        showToast(error.message || "Unable to cast vote.", "error");
    }
}

// ------------------------------------------------------
// ELECTION RESULTS
// ------------------------------------------------------

async function loadResults() {
    const table = getElement("results-table");
    const winnerContainer = getElement("winner-announcement");

    try {
        const data = await apiRequest("/api/results");

        const results = getArray(data, [
            "results",
            "candidates",
            "data"
        ]);

        if (results.length === 0) {
            table.innerHTML = `
                <tr>
                    <td colspan="5" class="empty-state">
                        No election results available yet.
                    </td>
                </tr>
            `;

            winnerContainer.innerHTML = "";
            return;
        }

        const sortedResults = results.slice().sort((a, b) => {
            return Number(b.votes ?? b.totalVotes ?? 0) -
                   Number(a.votes ?? a.totalVotes ?? 0);
        });

        const totalVotes = sortedResults.reduce((sum, candidate) => {
            return sum + Number(
                candidate.votes ?? candidate.totalVotes ?? 0
            );
        }, 0);

        table.innerHTML = sortedResults.map((candidate, index) => {
            const name = candidate.name ?? candidate.candidateName ?? "";
            const party = candidate.party ?? candidate.politicalParty ?? "";
            const votes = Number(
                candidate.votes ?? candidate.totalVotes ?? 0
            );

            const percentage = totalVotes > 0
                ? ((votes / totalVotes) * 100).toFixed(2)
                : "0.00";

            return `
                <tr>
                    <td>${index + 1}</td>
                    <td>${escapeHTML(name)}</td>
                    <td>${escapeHTML(party)}</td>
                    <td>${votes}</td>
                    <td>${percentage}%</td>
                </tr>
            `;
        }).join("");

        const highestVotes = Number(
            sortedResults[0].votes ??
            sortedResults[0].totalVotes ??
            0
        );

        const winners = sortedResults.filter(candidate => {
            return Number(
                candidate.votes ?? candidate.totalVotes ?? 0
            ) === highestVotes;
        });

        if (totalVotes === 0) {
            winnerContainer.innerHTML = "";
        } else if (winners.length === 1) {
            const winner = winners[0];

            winnerContainer.innerHTML = `
                <div class="winner-card">
                    <div class="winner-icon">🏆</div>

                    <div>
                        <h3>Leading Candidate: ${
                            escapeHTML(winner.name ?? winner.candidateName ?? "")
                        }</h3>

                        <p>
                            ${
                                escapeHTML(winner.party ?? winner.politicalParty ?? "")
                            }
                            — ${highestVotes} votes
                        </p>
                    </div>
                </div>
            `;
        } else {
            winnerContainer.innerHTML = `
                <div class="winner-card">
                    <div class="winner-icon">🏆</div>

                    <div>
                        <h3>Election is currently tied</h3>
                        <p>${highestVotes} votes each</p>
                    </div>
                </div>
            `;
        }

    } catch (error) {
        console.error("Results loading error:", error);

        table.innerHTML = `
            <tr>
                <td colspan="5" class="empty-state">
                    Unable to load election results.
                </td>
            </tr>
        `;

        winnerContainer.innerHTML = "";
    }
}

// ------------------------------------------------------
// EVENT LISTENERS
// ------------------------------------------------------

document.addEventListener("DOMContentLoaded", () => {
    // Sidebar navigation
    document.querySelectorAll(".nav-item").forEach(button => {
        button.addEventListener("click", () => {
            showPage(button.dataset.page);
        });
    });

    // Candidate registration
    getElement("candidate-form").addEventListener(
        "submit",
        registerCandidate
    );

    // Voter registration
    getElement("voter-form").addEventListener(
        "submit",
        registerVoter
    );

    // Voting
    getElement("vote-form").addEventListener(
        "submit",
        castVote
    );

    // Initial dashboard load
    loadDashboard();
});