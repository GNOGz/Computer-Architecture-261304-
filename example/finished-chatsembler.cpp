// lc2k_assembler.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cstdlib>
#include <limits>

using namespace std;

static const int MAXLINELENGTH = 1000;

// opcode map
static const unordered_map<string, int> opcodeMap = {
    {"add", 0},
    {"nand", 1}, // some docs call it "nor", project often uses "nand"/"nor" interchangeably; here we accept "nand"
    {"lw", 2},
    {"sw", 3},
    {"beq", 4},
    {"jalr", 5},
    {"halt", 6},
    {"noop", 7},
    {".fill", -1} // directive
};

bool isNumber(const string &s) {
    if (s.empty()) return false;
    // allow leading + or -
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (s.size() == 1) return false;
        i = 1;
    }
    for (; i < s.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

long parseNumber(const string &s) {
    // Parse as 32-bit signed, but use long for intermediate check
    long val = 0;
    try {
        size_t pos = 0;
        val = stol(s, &pos, 10);
        (void)pos;
    } catch (...) {
        cerr << "error: invalid number '" << s << "'\n";
        exit(1);
    }
    return val;
}

vector<string> tokenizeLine(const string &line) {
    // Remove comments: everything after '#' or ';' (common), but LC2K starters sometimes use comment after opcode
    string trimmed = line;
    // treat '#' as comment char and also '//' style if present
    size_t cpos = trimmed.find('#');
    if (cpos != string::npos) trimmed.erase(cpos);
    cpos = trimmed.find("//");
    if (cpos != string::npos) trimmed.erase(cpos);

    // Now split on whitespace
    vector<string> tokens;
    string token;
    istringstream ss(trimmed);
    while (ss >> token) tokens.push_back(token);
    return tokens;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "error: usage: " << argv[0] << " <assembly-code-file> <machine-code-file>\n";
        return 1;
    }

    string inFileString = argv[1];
    string outFileString = argv[2];

    ifstream inFile(inFileString);
    if (!inFile.is_open()) {
        cerr << "error in opening " << inFileString << "\n";
        return 1;
    }
    ofstream outFile(outFileString);
    if (!outFile.is_open()) {
        cerr << "error in opening " << outFileString << "\n";
        return 1;
    }

    // Read all lines first (we need line numbers)
    vector<string> lines;
    string line;
    while (getline(inFile, line)) {
        if (line.size() >= static_cast<size_t>(MAXLINELENGTH - 1)) {
            cerr << "error: line too long\n";
            return 1;
        }
        lines.push_back(line);
    }

    // PASS 1: build symbol table (label -> address)
    unordered_map<string, int> symtab;
    symtab.reserve(1024);
    for (size_t addr = 0; addr < lines.size(); ++addr) {
        const string &ln = lines[addr];
        vector<string> toks = tokenizeLine(ln);
        if (toks.empty()) continue;
        // If first token is an opcode or .fill, it is not a label.
        // Otherwise, first token is label.
        bool firstIsOpcode = (opcodeMap.find(toks[0]) != opcodeMap.end());
        string label;
        if (!firstIsOpcode) {
            label = toks[0];
            // ensure label isn't opcode name or numeric
            if (label.empty() || !isalpha(static_cast<unsigned char>(label[0]))) {
                cerr << "error: invalid label '" << label << "' at line " << addr << "\n";
                return 1;
            }
            if (symtab.find(label) != symtab.end()) {
                cerr << "error: duplicate label '" << label << "' at line " << addr << "\n";
                return 1;
            }
            symtab[label] = static_cast<int>(addr);
        }
    }

    // PASS 2: translate
    for (size_t addr = 0; addr < lines.size(); ++addr) {
        const string &ln = lines[addr];
        vector<string> toks = tokenizeLine(ln);
        if (toks.empty()) {
            // typically emit nothing for blank lines; but in LC2K every line corresponds to memory.
            // However the starter often expects blank lines removed. We'll emit nothing for blank lines.
            continue;
        }

        // Determine whether there's a label
        bool firstIsOpcode = (opcodeMap.find(toks[0]) != opcodeMap.end());
        string label, opcode;
        size_t idx = 0;
        if (firstIsOpcode) {
            opcode = toks[0];
            idx = 1;
        } else {
            if (toks.size() < 2) {
                cerr << "error: missing opcode at line " << addr << "\n";
                return 1;
            }
            label = toks[0];
            opcode = toks[1];
            idx = 2;
        }

        if (opcodeMap.find(opcode) == opcodeMap.end()) {
            cerr << "error: unrecognized opcode '" << opcode << "' at line " << addr << "\n";
            return 1;
        }

        int opc = opcodeMap.at(opcode);

        // helper to get token safely
        auto getTok = [&](size_t k)->string {
            if (idx + k < toks.size()) return toks[idx + k];
            return string();
        };

        auto parseReg = [&](const string &s)->int {
            if (!isNumber(s)) {
                cerr << "error: register must be a number 0..7, got '" << s << "' at line " << addr << "\n";
                exit(1);
            }
            long r = parseNumber(s);
            if (r < 0 || r > 7) {
                cerr << "error: register out of range 0..7, got " << r << " at line " << addr << "\n";
                exit(1);
            }
            return static_cast<int>(r);
        };

        int32_t machine = 0;

        if (opcode == "add" || opcode == "nand") {
            // R-type: opcode (3) | regA (3) | regB (3) | unused (13) | destReg (3)
            string sA = getTok(0), sB = getTok(1), sDest = getTok(2);
            if (sA.empty() || sB.empty() || sDest.empty()) {
                cerr << "error: wrong operand count for " << opcode << " at line " << addr << "\n";
                return 1;
            }
            int regA = parseReg(sA);
            int regB = parseReg(sB);
            int dest = parseReg(sDest);
            machine = (opc << 22) | (regA << 19) | (regB << 16) | dest;
            // dest occupies low 3 bits; bits 15..3 are unused (zero)
        } else if (opcode == "lw" || opcode == "sw" || opcode == "beq") {
            // I-type: opcode (3) | regA (3) | regB (3) | offsetField (16)
            string sA = getTok(0), sB = getTok(1), sOff = getTok(2);
            if (sA.empty() || sB.empty() || sOff.empty()) {
                cerr << "error: wrong operand count for " << opcode << " at line " << addr << "\n";
                return 1;
            }
            int regA = parseReg(sA);
            int regB = parseReg(sB);

            long offsetVal = 0;
            if (isNumber(sOff)) {
                offsetVal = parseNumber(sOff);
            } else {
                // label: for beq it's PC-relative; for lw/sw it's absolute address (label's address)
                auto it = symtab.find(sOff);
                if (it == symtab.end()) {
                    // undefined label -> treat as 0 (global undefined allowed)
                    offsetVal = 0;
                } else {
                    if (opcode == "beq") {
                        // offset = labelAddr - (PC + 1)
                        offsetVal = static_cast<long>(it->second) - static_cast<long>(addr) - 1;
                    } else {
                        // lw/sw offset field is a signed 16-bit immediate added to regA; in many projects
                        // they treat label as absolute address. We'll use label address.
                        offsetVal = static_cast<long>(it->second);
                    }
                }
            }

            // offset must fit in signed 16-bit
            if (offsetVal < -32768 || offsetVal > 32767) {
                cerr << "error: offset out of range at line " << addr << ": " << offsetVal << "\n";
                return 1;
            }

            // convert to unsigned 16-bit two's complement representation
            uint32_t offsetField = static_cast<uint32_t>(offsetVal) & 0xFFFF;
            machine = (opc << 22) | (regA << 19) | (regB << 16) | offsetField;
        } else if (opcode == "jalr") {
            // J-type: opcode (3) | regA (3) | regB (3) | unused (16)
            string sA = getTok(0), sB = getTok(1);
            if (sA.empty() || sB.empty()) {
                cerr << "error: wrong operand count for " << opcode << " at line " << addr << "\n";
                return 1;
            }
            int regA = parseReg(sA);
            int regB = parseReg(sB);
            machine = (opc << 22) | (regA << 19) | (regB << 16);
        } else if (opcode == "halt" || opcode == "noop") {
            // O-type: opcode (3) | unused (21)
            machine = (opc << 22);
        } else if (opcode == ".fill") {
            // .fill value or label
            string sVal = getTok(0);
            if (sVal.empty()) {
                cerr << "error: .fill missing operand at line " << addr << "\n";
                return 1;
            }
            long value = 0;
            if (isNumber(sVal)) {
                value = parseNumber(sVal);
            } else {
                auto it = symtab.find(sVal);
                if (it == symtab.end()) {
                    // undefined global label -> 0
                    value = 0;
                } else {
                    value = it->second;
                }
            }
            // machine word is just the 32-bit signed value (we will write it as decimal)
            machine = static_cast<int32_t>(value);
        } else {
            cerr << "error: unsupported opcode '" << opcode << "' at line " << addr << "\n";
            return 1;
        }

        // Write machine as signed decimal (one per line)
        outFile << static_cast<int32_t>(machine) << '\n';
    }

    inFile.close();
    outFile.close();
    return 0;
}
