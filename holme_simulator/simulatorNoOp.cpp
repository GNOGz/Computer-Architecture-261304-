#include <bits/stdc++.h>
using namespace std;

/* instruction-level simulator */
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit signed integer (sign-extend) */
    if (num & (1<<15)) {
        num -= (1<<16);
    }
    return num;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        return 1;
    }

    stateType state;
    state.pc = 0;
    state.numMemory = 0;
    // initialize registers and memory to 0
    for (int i = 0; i < NUMREGS; ++i) state.reg[i] = 0;
    for (int i = 0; i < NUMMEMORY; ++i) state.mem[i] = 0;

    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        printf("error: can't open file %s\n", argv[1]);
        return 1;
    }

    // read file into memory (each line contains a decimal integer instruction)
    string line;
    while (getline(infile, line)) {
        if (line.size() == 0) continue;
        // skip lines that are only whitespace
        bool allws = true;
        for (char c : line) if (!isspace((unsigned char)c)) { allws = false; break; }
        if (allws) continue;
        long long val;
        try {
            // allow negative numbers as well
            val = stoll(line);
        } catch (...) {
            printf("error in reading address %d\n", state.numMemory);
            return 1;
        }
        if (state.numMemory >= NUMMEMORY) {
            printf("error: program too big for memory\n");
            return 1;
        }
        state.mem[state.numMemory] = static_cast<int>(val);
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
        state.numMemory++;
    }
    infile.close();

    int instructionCount = 0;

    // Main loop: print state once BEFORE executing each instruction
    while (true) {
        

        if (state.pc < 0 || state.pc >= state.numMemory) {
            printf("ERROR: PC out of range (%d)\n", state.pc);
            return 1;
        }

        printState(&state);
        int instr = state.mem[state.pc];
        int opcode = (instr >> 22) & 0b111;      // top 3 bits in this simulator layout
        int regA   = (instr >> 19) & 0b111;
        int regB   = (instr >> 16) & 0b111;
        int destReg;
        int offsetField;
        int memoryAddress;

        switch (opcode) {
            case 0: // add: destReg = low 3 bits
                destReg = instr & 0x7;
                state.reg[destReg] = state.reg[regA] + state.reg[regB];
                state.pc++;
                instructionCount++;
                break;

            case 1: // nand
                destReg = instr & 0x7;
                state.reg[destReg] = ~(state.reg[regA] & state.reg[regB]);
                state.pc++;
                instructionCount++;
                break;

            case 2: // lw
                offsetField = instr & 0xFFFF; // low 16 bits
                offsetField = convertNum(offsetField);
                memoryAddress = state.reg[regA] + offsetField;
                if (memoryAddress < 0 || memoryAddress >= NUMMEMORY) {
                    printf("ERROR: Invalid memory address\n");
                    return 1;
                }
                state.reg[regB] = state.mem[memoryAddress];
                state.pc++;
                instructionCount++;
                break;

            case 3: // sw
                offsetField = instr & 0xFFFF;
                offsetField = convertNum(offsetField);
                memoryAddress = state.reg[regA] + offsetField;
                if (memoryAddress < 0 || memoryAddress >= NUMMEMORY) {
                    printf("ERROR: Invalid memory address\n");
                    return 1;
                }
                state.mem[memoryAddress] = state.reg[regB];
                state.pc++;
                instructionCount++;
                break;

            case 4: // beq
                offsetField = instr & 0xFFFF;
                offsetField = convertNum(offsetField);
                if (state.reg[regA] == state.reg[regB]) {
                    state.pc = state.pc + 1 + offsetField;
                } else {
                    state.pc++;
                }
                instructionCount++;
                break;

            case 5: // jalr
                // regA contains target, regB contains destination register for return address
                state.reg[regB] = state.pc + 1;
                if (state.reg[regA] == state.reg[regB]) {
                    state.pc++;
                } else {
                    state.pc = state.reg[regA];
                }
                instructionCount++;
                break;

            case 6: // halt
                instructionCount++;
                printf("machine halted\n");
                printf("total of %d instructions executed\n", instructionCount);
                printf("final state of machine:\n");
                state.pc++; // as specified in original behavior
                printState(&state);
                return 0;

            case 7: // noop
                state.pc++;
                instructionCount++;
                break;

            default:
                printf("ERROR: Memory at location %d is illegible.\n", state.pc);
                return 1;
        }
    }

    return 0;
}
