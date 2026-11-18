#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <algorithm>

#include <nlohmann/json.hpp>
#include "httplib.h"

// Include the Election classes from main.cpp.
// This prevents the console main() from being included
// when ELECTION_WEB_SERVER is defined in main.cpp.
#define ELECTION_WEB_SERVER
#include "../main.cpp"
#undef ELECTION_WEB_SERVER

using namespace std;
using json = nlohmann::json;

// ==================================================
// HELPER: SEND JSON RESPONSE
// ==================================================

void sendJSON(
    httplib::Response& res,
    const json& data,
    int status = 200
) {
    res.status = status;
    res.set_content(data.dump(4), "application/json");
}

// ==================================================
// HELPER: SEND ERROR RESPONSE
// ==================================================

void sendError(
    httplib::Response& res,
    const string& message,
    int status = 400
) {
    json response;

    response["success"] = false;
    response["message"] = message;

    sendJSON(res, response, status);
}

// ==================================================
// HELPER: CANDIDATE TO JSON
// ==================================================

json candidateToJSON(Candidate& candidate) {
    json data;

    data["id"] = candidate.getID();
    data["name"] = candidate.getName();
    data["party"] = candidate.getParty();
    data["votes"] = candidate.getVotes();

    return data;
}

// ==================================================
// HELPER: VOTER TO JSON
// ==================================================

json voterToJSON(Voter& voter) {
    json data;

    data["voterID"] = voter.getVoterID();
    data["name"] = voter.getName();
    data["hasVoted"] = voter.getVotingStatus();

    return data;
}

// ==================================================
// MAIN WEB SERVER
// ==================================================

int main() {

    Election election;

    // Load saved data from files.
    election.loadData();

    httplib::Server server;

    // ==========================================
// CORS CONFIGURATION
// ==========================================

const string allowedOrigin = "http://127.0.0.1:5500";

server.set_default_headers({
    {"Access-Control-Allow-Origin", allowedOrigin},
    {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
    {"Access-Control-Allow-Headers", "Content-Type"},
    {"Access-Control-Max-Age", "86400"}
});

// Handle preflight requests.
// Do not add Access-Control-Allow-Origin again here.
server.Options(
    R"(/.*)",
    [](const httplib::Request& req,
       httplib::Response& res) {

        res.status = 204;
    }
);

    // ==================================================
    // SERVE FRONTEND
    // ==================================================

    server.set_mount_point("/", "./frontend");

    // ==================================================
    // API: DASHBOARD STATISTICS
    // GET /api/dashboard
    // ==================================================

    server.Get(
        "/api/dashboard",
        [&](const httplib::Request& req,
            httplib::Response& res) {

            vector<Candidate> candidates =
                election.getCandidates();

            vector<Voter> voters =
                election.getVoters();

            int totalCandidates =
                static_cast<int>(candidates.size());

            int totalVoters =
                static_cast<int>(voters.size());

            int totalVotes =
                election.getTotalVotes();

            int votersVoted = 0;

            for (auto& voter : voters) {
                if (voter.getVotingStatus()) {
                    votersVoted++;
                }
            }

            json response;

            response["success"] = true;
            response["totalCandidates"] = totalCandidates;
            response["totalVoters"] = totalVoters;
            response["totalVotes"] = totalVotes;
            response["votersVoted"] = votersVoted;
            response["votersRemaining"] =
                totalVoters - votersVoted;

            sendJSON(res, response);
        }
    );

    // ==================================================
    // API: GET ALL CANDIDATES
    // GET /api/candidates
    // ==================================================

    server.Get(
        "/api/candidates",
        [&](const httplib::Request& req,
            httplib::Response& res) {

            json response;

            response["success"] = true;
            response["candidates"] = json::array();

            vector<Candidate> candidates =
                election.getCandidates();

            for (auto& candidate : candidates) {
                response["candidates"].push_back(
                    candidateToJSON(candidate)
                );
            }

            sendJSON(res, response);
        }
    );

    // ==================================================
    // API: REGISTER CANDIDATE
    // POST /api/candidates
    // ==================================================

    server.Post(
        "/api/candidates",
        [&](const httplib::Request& req,
            httplib::Response& res) {

            try {
                json body = json::parse(req.body);

                int id = body.at("id").get<int>();
                string name = body.at("name").get<string>();
                string party = body.at("party").get<string>();

                // Validate candidate details.
                if (
                    id <= 0 ||
                    name.empty() ||
                    party.empty()
                ) {
                    sendError(
                        res,
                        "Valid candidate ID, name, and party are required."
                    );
                    return;
                }

                // Check for duplicate candidate ID.
                vector<Candidate> candidates =
                    election.getCandidates();

                for (auto& candidate : candidates) {
                    if (candidate.getID() == id) {
                        sendError(
                            res,
                            "Candidate ID already exists.",
                            409
                        );
                        return;
                    }
                }

                // Register candidate.
                bool added =
                    election.addCandidate(id, name, party);

                if (!added) {
                    sendError(
                        res,
                        "Candidate registration failed.",
                        500
                    );
                    return;
                }

                json response;

                response["success"] = true;
                response["message"] =
                    "Candidate registered successfully.";

                sendJSON(res, response, 201);
            }
            catch (const json::exception& e) {
                sendError(
                    res,
                    "Invalid candidate data. Check the submitted fields."
                );
            }
            catch (const exception& e) {
                sendError(
                    res,
                    "Unable to register candidate.",
                    500
                );
            }
        }
    );

    // ==================================================
    // API: GET ALL VOTERS
    // GET /api/voters
    // ==================================================

    server.Get(
        "/api/voters",
        [&](const httplib::Request& req,
            httplib::Response& res) {

            json response;

            response["success"] = true;
            response["voters"] = json::array();

            vector<Voter> voters =
                election.getVoters();

            for (auto& voter : voters) {
                response["voters"].push_back(
                    voterToJSON(voter)
                );
            }

            sendJSON(res, response);
        }
    );

    // ==================================================
    // API: REGISTER VOTER
    // POST /api/voters
    // ==================================================

    server.Post(
        "/api/voters",
        [&](const httplib::Request& req,
            httplib::Response& res) {

            try {
                json body = json::parse(req.body);

                string name =
                    body.at("name").get<string>();

                string voterID =
                    body.at("voterID").get<string>();

                // Validate voter details.
                if (
                    name.empty() ||
                    voterID.empty()
                ) {
                    sendError(
                        res,
                        "Name and voter ID are required."
                    );
                    return;
                }

                // Check for duplicate voter ID.
                vector<Voter> voters =
                    election.getVoters();

                for (auto& voter : voters) {
                    if (voter.getVoterID() == voterID) {
                        sendError(
                            res,
                            "Voter ID already exists.",
                            409
                        );
                        return;
                    }
                }

                // Register voter.
                bool registered =
                    election.registerVoter(name, voterID);

                if (!registered) {
                    sendError(
                        res,
                        "Voter registration failed.",
                        500
                    );
                    return;
                }

                json response;

                response["success"] = true;
                response["message"] =
                    "Voter registered successfully.";

                sendJSON(res, response, 201);
            }
            catch (const json::exception& e) {
                sendError(
                    res,
                    "Invalid voter data. Check the submitted fields."
                );
            }
            catch (const exception& e) {
                sendError(
                    res,
                    "Unable to register voter.",
                    500
                );
            }
        }
    );

    // ==================================================
    // API: CAST VOTE
    // POST /api/vote
    // ==================================================

    server.Post(
        "/api/vote",
        [&](const httplib::Request& req,
            httplib::Response& res) {

            try {
                json body = json::parse(req.body);

                string voterID =
                    body.at("voterID").get<string>();

                int candidateID =
                    body.at("candidateID").get<int>();

                if (
                    voterID.empty() ||
                    candidateID <= 0
                ) {
                    sendError(
                        res,
                        "Valid voter ID and candidate ID are required."
                    );
                    return;
                }

                vector<Voter> voters =
                    election.getVoters();

                vector<Candidate> candidates =
                    election.getCandidates();

                bool voterFound = false;
                bool candidateFound = false;
                bool alreadyVoted = false;

                // Find voter.
                for (auto& voter : voters) {
                    if (voter.getVoterID() == voterID) {

                        voterFound = true;

                        if (voter.getVotingStatus()) {
                            alreadyVoted = true;
                        }

                        break;
                    }
                }

                // Find candidate.
                for (auto& candidate : candidates) {
                    if (candidate.getID() == candidateID) {
                        candidateFound = true;
                        break;
                    }
                }

                if (!voterFound) {
                    sendError(
                        res,
                        "Voter is not registered.",
                        404
                    );
                    return;
                }

                if (!candidateFound) {
                    sendError(
                        res,
                        "Candidate ID is invalid.",
                        404
                    );
                    return;
                }

                if (alreadyVoted) {
                    sendError(
                        res,
                        "This voter has already voted.",
                        409
                    );
                    return;
                }

                // Cast vote using Election class.
                bool voteSuccess =
                    election.castVote(voterID, candidateID);

                if (!voteSuccess) {
                    sendError(
                        res,
                        "Vote could not be recorded.",
                        400
                    );
                    return;
                }

                json response;

                response["success"] = true;
                response["message"] =
                    "Vote cast successfully.";

                sendJSON(res, response);
            }
            catch (const json::exception& e) {
                sendError(
                    res,
                    "Invalid voting data. Check the submitted fields."
                );
            }
            catch (const exception& e) {
                sendError(
                    res,
                    "Unable to process vote.",
                    500
                );
            }
        }
    );

    // ==================================================
    // API: ELECTION RESULTS
    // GET /api/results
    // ==================================================

    server.Get(
        "/api/results",
        [&](const httplib::Request& req,
            httplib::Response& res) {

            json response;

            response["success"] = true;
            response["results"] = json::array();

            vector<Candidate> candidates =
                election.getCandidates();

            int winnerVotes = -1;
            string winnerName = "";
            string winnerParty = "";

            for (auto& candidate : candidates) {

                response["results"].push_back(
                    candidateToJSON(candidate)
                );

                if (candidate.getVotes() > winnerVotes) {
                    winnerVotes = candidate.getVotes();
                    winnerName = candidate.getName();
                    winnerParty = candidate.getParty();
                }
            }

            if (candidates.empty()) {
                response["winner"] = nullptr;
            }
            else {
                response["winner"]["name"] = winnerName;
                response["winner"]["party"] = winnerParty;
                response["winner"]["votes"] = winnerVotes;
            }

            sendJSON(res, response);
        }
    );

    // ==================================================
    // SERVER START
    // ==================================================

    cout << "\n";
    cout << "====================================\n";
    cout << "   ELECTION MANAGEMENT WEB SERVER\n";
    cout << "====================================\n";

    cout << "\nServer running at:\n";
    cout << "http://127.0.0.1:8080\n";

    cout << "\nFrontend should be running at:\n";
    cout << "http://127.0.0.1:5500\n";

    cout << "\nPress CTRL + C to stop the server.\n";

    if (!server.listen("127.0.0.1", 8080)) {
        cerr << "\nFailed to start server!" << endl;
        return 1;
    }

    return 0;
}