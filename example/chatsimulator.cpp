#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#define NUMMEMORY 65536 // maximum number of words in memory
#define NUMREGS 8       // number of machine registers

struct State {
    int pc = 0;
    vector<int> mem;
    vector<int> reg;
    int numMemory = 0;
};

void printState(const State &state) {
    cout << "\n@@@\nstate:\n";
    cout << "\tpc " << state.pc << "\n";
    cout << "\tmemory:\n";
    for (int i = 0; i < state.numMemory; i++) {
        cout << "\t\tmem[" << i << "] " << state.mem[i] << "\n";
    }
    cout << "\tregisters:\n";
    for (int i = 0; i < NUMREGS; i++) {
        cout << "\t\treg[" << i << "] " << state.reg[i] << "\n";
    }
    cout << "end state\n";
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "error: usage: " << argv[0] << " <machine-code file>\n";
        return 1;
    }

    string filename = argv[1];
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "error: can't open file " << filename << endl;
        return 1;
    }

    State state;
    state.mem.resize(NUMMEMORY, 0);
    state.reg.resize(NUMREGS, 0);

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        int value;
        try {
            value = stoi(line);
        } catch (...) {
            cerr << "error in reading address " << state.numMemory << endl;
            return 1;
        }
        state.mem[state.numMemory] = value;
        cout << "memory[" << state.numMemory << "]=" << value << "\n";
        state.numMemory++;
    }

    // Just print the initial state (same as original)
    printState(state);

    return 0;
}
