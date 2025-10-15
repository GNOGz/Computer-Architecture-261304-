#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

#define NUMMEMORY 65536
#define NUMREGS 8

struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
};

typedef stateStruct stateType;

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
    if (num & (1 << 15)) {
        num -= (1 << 16);
    }
    return num;
}

struct Instruction {
    int opcode;
    int regA;
    int regB;
    int destOrOffset;
};

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
        stringstream ss(line);
        int value;
        if (!(ss >> value)) {
            cerr << "Error reading line " << index << endl;
            exit(1);
        }
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
    for (int &r : state.reg) r = 0;
}


void executeProgram(stateType &cpu, ofstream &outFile) {
    int instructionCount = 0;

    while (true) {
        printState(&cpu);
        fprintState(outFile, &cpu);

        Instruction inst = decodeInstruction(cpu.mem[cpu.pc]);

        switch (inst.opcode) {
            case 0: // add
                cpu.reg[inst.destOrOffset] = cpu.reg[inst.regA] + cpu.reg[inst.regB];
                break;

            case 1: // nor
                cpu.reg[inst.destOrOffset] = ~(cpu.reg[inst.regA] & cpu.reg[inst.regB]);
                break;

            case 2: // lw
                cpu.reg[inst.regB] = cpu.mem[cpu.reg[inst.regA] + inst.destOrOffset];
                break;

            case 3: // sw
                cpu.mem[cpu.reg[inst.regA] + inst.destOrOffset] = cpu.reg[inst.regB];
                break;

            case 4: // beq
                if (cpu.reg[inst.regA] == cpu.reg[inst.regB]) {
                    cpu.pc += inst.destOrOffset;
                }
                break;

            case 5: { // jalr
                int nextPC = cpu.pc + 1;
                cpu.pc = cpu.reg[inst.regA] - 1;
                cpu.reg[inst.regB] = nextPC;
                break;
            }

            case 6: // halt
                cout << "Machine halted.\nTotal of " << (instructionCount + 1)
                     << " instructions executed.\nFinal state of machine:\n";
                outFile << "Machine halted.\nTotal of " << (instructionCount + 1)
                        << " instructions executed.\nFinal state of machine:\n";

                cpu.pc++;
                printState(&cpu);
                fprintState(outFile, &cpu);

                cout << "\nRegister 1 holds: " << cpu.reg[1] << "\n";
                outFile << "\nRegister 1 holds: " << cpu.reg[1] << "\n";
                return;

            case 7: // noop
                break;

            default:
                cerr << "Error: Invalid opcode " << inst.opcode << endl;
                exit(1);
        }

        cpu.pc++;
        instructionCount++;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        cerr << "Usage: " << argv[0] << " <machine-code file> [output file]\n";
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = (argc == 3) ? argv[2] : "output.txt"; // default file

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
