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

//print the state to an output file (for easier debugging )
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

Instruction decodeInstruction(int machineCode) {
    Instruction inst{};
    inst.opcode = (machineCode >> 22) & 0x7;
    inst.regA   = (machineCode >> 19) & 0x7;
    inst.regB   = (machineCode >> 16) & 0x7;

    if (inst.opcode == 0 || inst.opcode == 1) {
        inst.destOrOffset = machineCode & 0x7;
        // R-type: add (0), nor (1) -> dest is low 3 bits
        inst.destOrOffset = machineCode & 0x7; // 3-bit dest (0..7)
    } else {
        // I-type or other: lower 16 bits are offset (signed)
        inst.destOrOffset = signExtend16(machineCode & 0xFFFF);
    }
    return inst;
}


//read in the input file (machine-code file) into memory and also print it out to terminal and output file
void loadProgram(const string &filename, stateType &state, ofstream &outFile) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: cannot open input file " << filename << endl;
        exit(1);
    }

    string line;
    int index = 0;

    while (getline(file, line)) {
        //check line length
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
        state.mem[index++] = value;
    }

    state.numMemory = index;

    for (int i = 0; i < state.numMemory; i++) {
        cout << "memory[" << i << "]=" << state.mem[i] << endl;
        outFile << "memory[" << i << "]=" << state.mem[i] << endl;
    }
}


void initializeMachine(stateType &state) {
    // state.pc = 0;
    // for (int &r : state.reg) r = 0;
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
        printState(&cpu);
        fprintState(outFile, &cpu);

        Instruction inst = decodeInstruction(cpu.mem[cpu.pc]);

        switch (inst.opcode) {
            case 0: // add
                cpu.reg[inst.destOrOffset] = cpu.reg[inst.regA] + cpu.reg[inst.regB];
                break;

            case 1: // nand
                cpu.reg[inst.destOrOffset] = ~(cpu.reg[inst.regA] & cpu.reg[inst.regB]);
                break;

            case 2: // lw
                int addr = cpu.mem[cpu.reg[inst.regA] + inst.destOrOffset];
                if(addr < 0 ||  addr >= NUMMEMORY){//check Error: 'lw' address out of bounds 
                    cout << "Error: 'lw' address out of bounds: [ "<< addr << "\n";
                    break;
                }
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
                int nextPC = cpu.pc + 1;       // store PC+1 (next instruction) before modifying PC
                cpu.reg[inst.regB] = nextPC;   // save nextPC into regB, even if regA == regB
                cpu.pc = cpu.reg[inst.regA];   // jump to the address stored in regA
                cpu.pc--;                       // subtract 1 because PC will be incremented after the switch-case
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