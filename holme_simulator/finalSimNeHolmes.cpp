#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8       /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

struct Instruction {
    int opcode;
    int regA;
    int regB;
    int destOrOffset;
};

void printState(stateType *);
void fprintState(ofstream &out, stateType *statePtr);
int signExtend16(int num);
void loadProgram(const string &filename, stateType &state, ofstream &outFile);
Instruction decodeInstruction(int machineCode);
void initializeMachine(stateType &state);
void executeProgram(stateType &cpu, ofstream &outFile);

// --- helper for safe memory/register access ---
inline void checkMemAccess(int addr) {
    if (addr < 0 || addr >= NUMMEMORY) {
        cerr << "Memory access out of bounds: address " << addr << endl;
        exit(1);
    }
}

inline void checkRegAccess(int reg) {
    if (reg < 0 || reg >= NUMREGS) {
        cerr << "Register access out of bounds: register " << reg << endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        cerr << "Usage: " << argv[0] << " <machine-code file> [output file]\n";
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = (argc == 3) ? argv[2] : "output.txt";

    ofstream out(outputFile);
    if (!out.is_open()) {
        cerr << "Error: cannot open output file " << outputFile << endl;
        return 1;
    }

    stateType machine{};
    initializeMachine(machine);
    loadProgram(inputFile, machine, out);
    executeProgram(machine, out);
    out.close();
    return 0;
}

void printState(stateType *statePtr) {
    cout << "\n@@@\nstate:\n";
    cout << "\tpc " << statePtr->pc << "\n";
    cout << "\tmemory:\n";
    for (int i = 0; i < statePtr->numMemory; i++) {
        cout << "\t\tmem[ " << i << " ] " << statePtr->mem[i] << "\n";
    }
    cout << "\tregisters:\n";
    for (int i = 0; i < NUMREGS; i++) {
        cout << "\t\treg[ " << i << " ] " << statePtr->reg[i] << "\n";
    }
    cout << "end state\n";
}

void fprintState(ofstream &out, stateType *statePtr) {
    out << "\n@@@\nstate:\n";
    out << "\tpc " << statePtr->pc << "\n";
    out << "\tmemory:\n";
    for (int i = 0; i < statePtr->numMemory; i++) {
        out << "\t\tmem[ " << i << " ] " << statePtr->mem[i] << "\n";
    }
    out << "\tregisters:\n";
    for (int i = 0; i < NUMREGS; i++) {
        out << "\t\treg[ " << i << " ] " << statePtr->reg[i] << "\n";
    }
    out << "end state\n";
}

int signExtend16(int num) {
    if (num & (1 << 15)) num -= (1 << 16);
    return num;
}

Instruction decodeInstruction(int machineCode) {
    Instruction inst{};
    inst.opcode = (machineCode >> 22) & 0x7;
    inst.regA = (machineCode >> 19) & 0x7;
    inst.regB = (machineCode >> 16) & 0x7;
    inst.destOrOffset = signExtend16(machineCode & 0xFFFF);
    return inst;
}

void loadProgram(const string &filename, stateType &state, ofstream &outFile) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: cannot open input file " << filename << endl;
        exit(1);
    }

    string line;
    int index = 0;

    while (getline(file, line)) {
        if (line.length() > MAXLINELENGTH) {
            cerr << "Error: line " << index
                 << " exceeds MAXLINELENGTH of " << MAXLINELENGTH << endl;
            exit(1);
        }

        stringstream ss(line);
        int value;
        if (!(ss >> value)) {
            cerr << "Error reading line " << index << endl;
            exit(1);
        }

        checkMemAccess(index);
        state.mem[index++] = value;
    }

    state.numMemory = index;
    for (int i = 0; i < state.numMemory; i++) {
        cout << "memory[" << i << "]=" << state.mem[i] << endl;
        outFile << "memory[" << i << "]=" << state.mem[i] << endl;
    }
}

void initializeMachine(stateType &state) {
    state.pc = 0;
    for (int i = 0; i < NUMREGS; ++i)
        state.reg[i] = 0;
    state.numMemory = 0;
    for (int i = 0; i < NUMMEMORY; ++i)
        state.mem[i] = 0;
}
void executeProgram(stateType &cpu, ofstream &outFile) {
    int instructionCount = 0;

    while (true) {
        checkMemAccess(cpu.pc);

        printState(&cpu);
        fprintState(outFile, &cpu);

        Instruction inst = decodeInstruction(cpu.mem[cpu.pc]);

        // always validate regA and regB
        checkRegAccess(inst.regA);
        checkRegAccess(inst.regB);

        switch (inst.opcode) {
            case 0: // add
                checkRegAccess(inst.destOrOffset);
                cpu.reg[inst.destOrOffset] =
                    cpu.reg[inst.regA] + cpu.reg[inst.regB];
                break;

            case 1: // nor
                checkRegAccess(inst.destOrOffset);
                cpu.reg[inst.destOrOffset] =
                    ~(cpu.reg[inst.regA] | cpu.reg[inst.regB]);
                break;

            case 2: { // lw
                int addr = cpu.reg[inst.regA] + inst.destOrOffset;
                checkMemAccess(addr);
                checkRegAccess(inst.regB);
                cpu.reg[inst.regB] = cpu.mem[addr];
                break;
            }

            case 3: { // sw
                int addr = cpu.reg[inst.regA] + inst.destOrOffset;
                checkMemAccess(addr);
                checkRegAccess(inst.regB);
                cpu.mem[addr] = cpu.reg[inst.regB];
                break;
            }

            case 4: { // beq
                if (cpu.reg[inst.regA] == cpu.reg[inst.regB]) {
                    int newPc = cpu.pc + 1 + inst.destOrOffset;
                    if (newPc < 0 || newPc >= NUMMEMORY) {
                        cerr << "Memory access out of bounds (branch target): " << newPc << endl;
                        exit(1);
                    }
                    cpu.pc = newPc;
                    instructionCount++;
                    continue; // skip pc increment
                }
                break;
            }

            case 5: { // jalr
                int nextPC = cpu.pc + 1;
                int newPc = cpu.reg[inst.regA];
                checkRegAccess(inst.regB);
                if (newPc < 0 || newPc >= NUMMEMORY) {
                    cerr << "Memory access out of bounds (jalr target): " << newPc << endl;
                    exit(1);
                }
                cpu.reg[inst.regB] = nextPC;
                cpu.pc = newPc;
                instructionCount++;
                continue;
            }

            case 6: // halt
                cout << "Machine halted.\nTotal of " << (instructionCount + 1)
                     << " instructions executed.\nFinal state of machine:\n";
                outFile << "Machine halted.\nTotal of " << (instructionCount + 1)
                        << " instructions executed.\nFinal state of machine:\n";
                printState(&cpu);
                fprintState(outFile, &cpu);
                return;

            case 7: // noop
                break;

            default:
                cerr << "Error: Invalid opcode " << inst.opcode << endl;
                exit(1);
        }

        cpu.pc++;
        instructionCount++;

        if (cpu.pc < 0 || cpu.pc >= NUMMEMORY) {
            cerr << "Memory access out of bounds (next PC): " << cpu.pc << endl;
            exit(1);
        }
    }
}
