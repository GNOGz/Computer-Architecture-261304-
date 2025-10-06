#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

#define MAXLINELENGTH 1000

using namespace std;

bool readAndParse(ifstream &inFile, string &label, string &opcode,
                  string &arg0, string &arg1, string &arg2);
bool isNumber(const string &s);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "error: usage: " << argv[0] << " <assembly-code-file> <machine-code-file>\n";
        exit(1);
    }

    string inFileString = argv[1];
    string outFileString = argv[2];

    ifstream inFile(inFileString);
    if (!inFile.is_open())
    {
        cerr << "error in opening " << inFileString << "\n";
        exit(1);
    }

    ofstream outFile(outFileString);
    if (!outFile.is_open())
    {
        cerr << "error in opening " << outFileString << "\n";
        exit(1);
    }

    string label, opcode, arg0, arg1, arg2;

    // Example: how to use readAndParse
    if (!readAndParse(inFile, label, opcode, arg0, arg1, arg2))
    {
        // reached end of file
    }

    // rewind file (start reading again)
    inFile.clear();
    inFile.seekg(0, ios::beg);

    // test opcode
    if (opcode == "add")
    {
        // handle opcode "add"
    }

    inFile.close();
    outFile.close();

    return 0;
}

/*
 * Read and parse a line of the assembly-language file.
 * Returns false if reached EOF, true otherwise.
 */
bool readAndParse(ifstream &inFile, string &label, string &opcode,
                  string &arg0, string &arg1, string &arg2)
{
    string line;
    if (!getline(inFile, line))
        return false; // EOF

    if (line.size() >= MAXLINELENGTH - 1)
    {
        cerr << "error: line too long\n";
        exit(1);
    }

    label.clear();
    opcode.clear();
    arg0.clear();
    arg1.clear();
    arg2.clear();

    stringstream ss(line);
    ss >> label >> opcode >> arg0 >> arg1 >> arg2;

    // Handle case where no label exists
    // (If the first token is an opcode, shift it left)
    if (!opcode.empty() && opcode[0] != '\0' && 
        (opcode == "add" || opcode == "nand" || opcode == "lw" || opcode == "sw" ||
         opcode == "beq" || opcode == "jalr" || opcode == "halt" || opcode == "noop" || opcode == ".fill"))
    {
        // It's actually the opcode, not label
        arg2 = arg1;
        arg1 = arg0;
        arg0 = opcode;
        opcode = label;
        label.clear();
    }

    return true;
}

bool isNumber(const string &s)
{
    if (s.empty())
        return false;
    char *endptr = nullptr;
    strtol(s.c_str(), &endptr, 10);
    return (*endptr == '\0');
}
