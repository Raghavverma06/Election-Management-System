
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace std;

// ==========================================
// CLASS 1: PERSON
// ==========================================

class Person {
protected:
    string name;

public:
    Person(string n = "") : name(n) {}

    virtual void display() const {
        cout << "Name: " << name << endl;
    }

    string getName() const {
        return name;
    }

    virtual ~Person() {}
};

// ==========================================
// CLASS 2: CANDIDATE
// ==========================================

class Candidate : public Person {
private:
    int candidateID;
    string party;
    int votes;

public:
    Candidate(int id = 0, string n = "", string p = "")
        : Person(n), candidateID(id), party(p), votes(0) {}

    int getID() const {
        return candidateID;
    }

    string getParty() const {
        return party;
    }

    int getVotes() const {
        return votes;
    }

    void addVote() {
        votes++;
    }

    void display() const override {
        cout << left
             << setw(12) << candidateID
             << setw(20) << name
             << setw(20) << party
             << setw(10) << votes
             << endl;
    }
};

// ==========================================
// CLASS 3: VOTER
// ==========================================

class Voter : public Person {
private:
    string voterID;
    bool hasVoted;

public:
    Voter(string n = "", string id = "")
        : Person(n), voterID(id), hasVoted(false) {}

    string getVoterID() const {
        return voterID;
    }

    bool getVotingStatus() const {
        return hasVoted;
    }

    void markVoted() {
        hasVoted = true;
    }

    void display() const override {
        cout << left
             << setw(20) << voterID
             << setw(25) << name
             << setw(15)
             << (hasVoted ? "Voted" : "Not Voted")
             << endl;
    }
};

// ==========================================
// CLASS 4: ADMIN
// ==========================================

class Admin : public Person {
private:
    string adminID;

public:
    Admin(string n = "", string id = "")
        : Person(n), adminID(id) {}

    bool login(string username, string password) const {
        return username == "admin" && password == "1234";
    }

    void display() const override {
        cout << "\nWelcome, Administrator "
             << name << "!" << endl;

        cout << "Admin ID: " << adminID << endl;
    }
};

// ==========================================
// CLASS 5: ELECTION
// ==========================================

class Election {
private:
    vector<Candidate> candidates;
    vector<Voter> voters;

    // Find candidate by ID
    int findCandidate(int id) const {
        for (size_t i = 0; i < candidates.size(); i++) {
            if (candidates[i].getID() == id) {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    // Find voter by ID
    int findVoter(const string& id) const {
        for (size_t i = 0; i < voters.size(); i++) {
            if (voters[i].getVoterID() == id) {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

public:

    // --------------------------------------
    // REGISTER CANDIDATE
    // --------------------------------------

    bool addCandidate(int id, string name, string party) {
        if (id <= 0 || name.empty() || party.empty()) {
            cout << "\nInvalid candidate details!" << endl;
            return false;
        }

        if (findCandidate(id) != -1) {
            cout << "\nCandidate ID already exists!" << endl;
            return false;
        }

        candidates.push_back(Candidate(id, name, party));

        saveData();

        cout << "\nCandidate registered successfully!" << endl;
        return true;
    }

    // --------------------------------------
    // REGISTER VOTER
    // --------------------------------------

    bool registerVoter(string name, string voterID) {
        if (name.empty() || voterID.empty()) {
            cout << "\nInvalid voter details!" << endl;
            return false;
        }

        if (findVoter(voterID) != -1) {
            cout << "\nVoter ID already exists!" << endl;
            return false;
        }

        voters.push_back(Voter(name, voterID));

        saveData();

        cout << "\nVoter registered successfully!" << endl;
        return true;
    }

    // --------------------------------------
    // DISPLAY CANDIDATES
    // --------------------------------------

    void displayCandidates() const {
        if (candidates.empty()) {
            cout << "\nNo candidates registered!" << endl;
            return;
        }

        cout << "\n========== CANDIDATE LIST ==========\n";

        cout << left
             << setw(12) << "ID"
             << setw(20) << "Name"
             << setw(20) << "Party"
             << setw(10) << "Votes"
             << endl;

        cout << string(62, '-') << endl;

        for (const auto& candidate : candidates) {
            candidate.display();
        }
    }

    // --------------------------------------
    // DISPLAY VOTERS
    // --------------------------------------

    void displayVoters() const {
        if (voters.empty()) {
            cout << "\nNo voters registered!" << endl;
            return;
        }

        cout << "\n========== VOTER LIST ==========\n";

        cout << left
             << setw(20) << "Voter ID"
             << setw(25) << "Name"
             << setw(15) << "Status"
             << endl;

        cout << string(60, '-') << endl;

        for (const auto& voter : voters) {
            voter.display();
        }
    }

    // --------------------------------------
    // SEARCH CANDIDATE
    // --------------------------------------

    void searchCandidate(int id) const {
        int index = findCandidate(id);

        if (index == -1) {
            cout << "\nCandidate not found!" << endl;
            return;
        }

        const Candidate& candidate = candidates[index];

        cout << "\n========== CANDIDATE FOUND ==========\n";

        cout << "Candidate ID: " << candidate.getID() << endl;
        cout << "Name: " << candidate.getName() << endl;
        cout << "Party: " << candidate.getParty() << endl;
        cout << "Votes: " << candidate.getVotes() << endl;
    }

    // --------------------------------------
    // CAST VOTE
    // --------------------------------------

    bool castVote(string voterID, int candidateID) {
        int voterIndex = findVoter(voterID);
        int candidateIndex = findCandidate(candidateID);

        if (voterIndex == -1) {
            cout << "\nVoter not registered!" << endl;
            return false;
        }

        if (candidateIndex == -1) {
            cout << "\nInvalid Candidate ID!" << endl;
            return false;
        }

        if (voters[voterIndex].getVotingStatus()) {
            cout << "\nYou have already voted!" << endl;
            return false;
        }

        candidates[candidateIndex].addVote();
        voters[voterIndex].markVoted();

        saveData();

        cout << "\nYour vote has been cast successfully!" << endl;
        return true;
    }

    // --------------------------------------
    // DISPLAY ELECTION RESULTS
    // --------------------------------------

    void displayResults() const {
        if (candidates.empty()) {
            cout << "\nNo candidates available!" << endl;
            return;
        }

        cout << "\n========== ELECTION RESULTS ==========\n";

        cout << left
             << setw(12) << "ID"
             << setw(20) << "Name"
             << setw(20) << "Party"
             << setw(10) << "Votes"
             << endl;

        cout << string(62, '-') << endl;

        for (const auto& candidate : candidates) {
            candidate.display();
        }

        int winnerIndex = 0;

        for (size_t i = 1; i < candidates.size(); i++) {
            if (candidates[i].getVotes() >
                candidates[winnerIndex].getVotes()) {
                winnerIndex = static_cast<int>(i);
            }
        }

        cout << "\n========== WINNER ==========\n";

        cout << "Winner: "
             << candidates[winnerIndex].getName()
             << endl;

        cout << "Party: "
             << candidates[winnerIndex].getParty()
             << endl;

        cout << "Total Votes: "
             << candidates[winnerIndex].getVotes()
             << endl;
    }

    // --------------------------------------
    // SAVE DATA TO FILE
    // --------------------------------------

    void saveData() const {
        ofstream candidateFile("candidates.txt");

        if (!candidateFile) {
            cerr << "Unable to open candidates.txt" << endl;
            return;
        }

        for (const auto& candidate : candidates) {
            candidateFile
                << candidate.getID() << "|"
                << candidate.getName() << "|"
                << candidate.getParty() << "|"
                << candidate.getVotes()
                << endl;
        }

        candidateFile.close();

        ofstream voterFile("voters.txt");

        if (!voterFile) {
            cerr << "Unable to open voters.txt" << endl;
            return;
        }

        for (const auto& voter : voters) {
            voterFile
                << voter.getVoterID() << "|"
                << voter.getName() << "|"
                << voter.getVotingStatus()
                << endl;
        }

        voterFile.close();
    }

    // --------------------------------------
    // LOAD DATA FROM FILE
    // --------------------------------------

    void loadData() {
        candidates.clear();
        voters.clear();

        // Load candidates
        ifstream candidateFile("candidates.txt");
        string line;

        while (getline(candidateFile, line)) {
            stringstream ss(line);

            string idText;
            string name;
            string party;
            string votesText;

            if (!getline(ss, idText, '|') ||
                !getline(ss, name, '|') ||
                !getline(ss, party, '|') ||
                !getline(ss, votesText)) {
                continue;
            }

            try {
                int id = stoi(idText);
                int votes = stoi(votesText);

                if (id <= 0 || votes < 0) {
                    continue;
                }

                Candidate candidate(id, name, party);

                for (int i = 0; i < votes; i++) {
                    candidate.addVote();
                }

                candidates.push_back(candidate);
            }
            catch (const exception&) {
                continue;
            }
        }

        candidateFile.close();

        // Load voters
        ifstream voterFile("voters.txt");

        while (getline(voterFile, line)) {
            stringstream ss(line);

            string voterID;
            string name;
            string statusText;

            if (!getline(ss, voterID, '|') ||
                !getline(ss, name, '|') ||
                !getline(ss, statusText)) {
                continue;
            }

            Voter voter(name, voterID);

            if (statusText == "1") {
                voter.markVoted();
            }

            voters.push_back(voter);
        }

        voterFile.close();
    }

    // --------------------------------------
    // DISPLAY DASHBOARD
    // --------------------------------------

    void displayDashboard() const {
        int totalCandidates =
            static_cast<int>(candidates.size());

        int totalVoters =
            static_cast<int>(voters.size());

        int totalVotes = getTotalVotes();

        cout << "\n";
        cout << "========================================\n";
        cout << "          ELECTION DASHBOARD            \n";
        cout << "========================================\n";

        cout << "\nTotal Candidates : " << totalCandidates << endl;
        cout << "Total Voters     : " << totalVoters << endl;
        cout << "Total Votes Cast : " << totalVotes << endl;

        cout << "\n----------------------------------------\n";
        cout << "         CANDIDATE VOTE SUMMARY         \n";
        cout << "----------------------------------------\n";

        for (const auto& candidate : candidates) {
            cout << candidate.getName()
                 << " (" << candidate.getParty() << ")"
                 << " : "
                 << candidate.getVotes()
                 << " votes"
                 << endl;
        }

        cout << "========================================\n";
    }

    // --------------------------------------
    // GET ALL CANDIDATES
    // --------------------------------------

    vector<Candidate> getCandidates() const {
        return candidates;
    }

    // --------------------------------------
    // GET ALL VOTERS
    // --------------------------------------

    vector<Voter> getVoters() const {
        return voters;
    }

    // --------------------------------------
    // GET TOTAL VOTES
    // --------------------------------------

    int getTotalVotes() const {
        int total = 0;

        for (const auto& candidate : candidates) {
            total += candidate.getVotes();
        }

        return total;
    }
};

// ==========================================
// CONSOLE MAIN FUNCTION
// ==========================================

// The web server defines ELECTION_WEB_SERVER before
// including this file, so this console main is excluded.

#ifndef ELECTION_WEB_SERVER

int main() {
    Election election;

    Admin admin("Raghav", "ADMIN101");

    election.loadData();

    int choice = 0;
    string name;
    string voterID;
    string party;
    int candidateID = 0;

    cout << "========================================\n";
    cout << "       ELECTION MANAGEMENT SYSTEM       \n";
    cout << "========================================\n";

    // Admin login
    string username;
    string password;

    cout << "\n========== ADMIN LOGIN ==========\n";

    cout << "Username: ";
    cin >> username;

    cout << "Password: ";
    cin >> password;

    if (!admin.login(username, password)) {
        cout << "\nInvalid Login!" << endl;
        return 0;
    }

    admin.display();

    // Main menu
    do {
        cout << "\n========== MAIN MENU ==========\n";

        cout << "1. Register Candidate\n";
        cout << "2. Register Voter\n";
        cout << "3. Display Candidates\n";
        cout << "4. Display Voters\n";
        cout << "5. Cast Vote\n";
        cout << "6. Display Election Results\n";
        cout << "7. Search Candidate\n";
        cout << "8. Election Dashboard\n";
        cout << "9. Exit\n";

        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );

            cout << "\nInvalid input. Enter a number." << endl;
            continue;
        }

        switch (choice) {
        case 1:
            cout << "\nEnter Candidate ID: ";
            cin >> candidateID;

            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );

            cout << "Enter Candidate Name: ";
            getline(cin, name);

            cout << "Enter Party Name: ";
            getline(cin, party);

            election.addCandidate(candidateID, name, party);
            break;

        case 2:
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );

            cout << "\nEnter Voter Name: ";
            getline(cin, name);

            cout << "Enter Voter ID: ";
            getline(cin, voterID);

            election.registerVoter(name, voterID);
            break;

        case 3:
            election.displayCandidates();
            break;

        case 4:
            election.displayVoters();
            break;

        case 5:
            cout << "\nEnter Voter ID: ";
            cin >> voterID;

            election.displayCandidates();

            cout << "\nEnter Candidate ID: ";
            cin >> candidateID;

            election.castVote(voterID, candidateID);
            break;

        case 6:
            election.displayResults();
            break;

        case 7:
            cout << "\nEnter Candidate ID to Search: ";
            cin >> candidateID;

            election.searchCandidate(candidateID);
            break;

        case 8:
            election.displayDashboard();
            break;

        case 9:
            cout << "\nExiting Election Management System..."
                 << endl;
            break;

        default:
            cout << "\nInvalid Choice! Try Again." << endl;
        }

    } while (choice != 9);

    return 0;
}

#endif
