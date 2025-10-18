#include <bits/stdc++.h>
#include <string>

using namespace std;

// map to store opcode,address of label and operationType
map<string, string> operationType;
map<string, string> opcode;
map<string, int> addressOfLabel;

string stream; // global variable to store each operation in each line

bool isNumber(string);
void initParser();
vector<string> readAllLines(string);
string tokenize();
int convertToNumber(string);
string numberToBinary(int);
string numberToBinaryI(int);
string numberToBinary32(int);
int binaryStringToDem(string);
bool validateLabel(string);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    vector<string> line = readAllLines(argv[1]);
    freopen(argv[2], "w", stdout);
    vector<string> binaryResult;
    initParser();
    for (int i = 0; i < line.size(); i++)
    { // first loop is to loop through all lines and save the address of the label.
        // check whitespace to see if it has label or not
        stream = line[i];
        // cout << line[i] << endl;
        if (stream[0] != ' ' && stream[0] != '\0' && stream[0] != '\t')
        { // label found since it is not white space in the start
            string labelName = tokenize();
            if (validateLabel(labelName))
                addressOfLabel[labelName] = i; // save label name to map
            continue;
        }
    }

    // for (auto [a, b] : addressOfLabel){
    //     cout << "label name: " << a << "|| address of label : " << b << endl;
    // } // uncomment to debug address list

    for (int i = 0; i < line.size(); i++)
    { // second loop is for creating opcode
        stream = line[i];
        // cout << line[i] << endl;
        if (stream[0] != ' ' && stream[0] != '\0' && stream[0] != '\t')
        {               // label found since it is not white space in the start
            tokenize(); // tokenize right away to get rid of label
        }
        string operation = tokenize();
        string binary = "0000000";
        string regA, regB, regDes, offset;
        string valRegA, valRegB, valRegDes, ValOffest;
        bool isFill = 0;
        binary += opcode[operation];
        // cout << operation << endl;
        if (operationType[operation] == "R")
        { // 3 fields
            // R
            regA = tokenize();
            regB = tokenize();
            regDes = tokenize();
            valRegA = numberToBinary(convertToNumber(regA));
            valRegB = numberToBinary(convertToNumber(regB));
            valRegDes = numberToBinary(convertToNumber(regDes));
            binary += valRegA + valRegB + "0000000000000" + valRegDes;
        }
        else if (operationType[operation] == "I")
        { // 3 fields
            // I check
            regA = tokenize();
            regB = tokenize();
            offset = tokenize();
            valRegA = numberToBinary(convertToNumber(regA));
            valRegB = numberToBinary(convertToNumber(regB));
            if (operation == "beq" && !isNumber(offset))
            {
                // cout << convertToNumber(offset) - i -1 << '\n';
                ValOffest = numberToBinaryI(convertToNumber(offset) - i - 1);
            }
            else
            {
                ValOffest = numberToBinaryI(convertToNumber(offset));
            }
            binary += valRegA + valRegB + ValOffest;
        }
        else if (operationType[operation] == "J")
        { // 2 fields
            regA = tokenize();
            regB = tokenize();
            valRegA = numberToBinary(convertToNumber(regA));
            valRegB = numberToBinary(convertToNumber(regB));
            binary += valRegA + valRegB + "0000000000000000";
        }
        else if (operationType[operation] == "O")
        { // no field
            while (binary.size() < 32)
            {
                binary += "0";
            }
        }
        else if (operation == ".fill")
        {
            string value = tokenize();
            // cout << convertToNumber(value)<< " "<< binaryStringToDem(numberToBinary32(convertToNumber(value))) << endl;
            binaryResult.push_back(numberToBinary32(convertToNumber(value)));
            isFill = 1;
        }
        else
        {
            cerr << "Unexpected operation: " + operation + "\nat line " + to_string(i);
            exit(1);
        }
        // cout << "Debugging: " << operation << ' ' << regA << ' ' << regB << ' ' << offset << '\n';
        // cout << "Num: " << valRegA << ' ' << valRegB << ' ' << ValOffest << '\n';
        // cout << "Dec: " << binaryStringToDem(valRegA) << ' ' << binaryStringToDem(valRegB) << ' ' <<binaryStringToDem( ValOffest) << '\n';
        if (!isFill)
            binaryResult.push_back(binary);
    }

    for (int i = 0; i < binaryResult.size(); i++)
    {
        // cout<<"addresss " << i <<" " <<binaryResult[i]  << "|| size : " << binaryResult[i].size() << endl;
        // cout <<"address " << i << ": " << binaryStringToDem(binaryResult[i])<< " || "<<line[i] << endl;
        cout << binaryStringToDem(binaryResult[i]) << endl;
    }
    exit(0);
}

vector<string> readAllLines(string path)
{
    ifstream file(path); // open your file
    if (!file.is_open())
    {
        cerr << "can not open file";
        exit(1);
    }

    vector<string> lines;
    string line;

    while (getline(file, line))
    {                          // read line by line
        lines.push_back(line); // store each line in the vector
    }

    file.close();
    return lines;
}

bool isNumber(string str)
{
    if (str.empty())
        return false;

    int start = 0;

    if (str[0] == '-')
    {
        if (str.size() == 1)
            return false;
        start = 1;
    }

    for (int i = start; i < str.size(); ++i)
    {
        if (!std::isdigit(str[i]))
            return false;
    }

    return true;
}

string tokenize()
{
    // cout << "tokenizing: " << stream << endl;
    string tokenToReturn = "";
    int i = 0;
    while (i < stream.size())
    {
        while (stream[i] == ' ' || stream[i] == '\0' || stream[i] == '\t')
        {
            i++;
        }

        while (stream[i] != ' ' && stream[i] != '\0' && stream[i] != '\t')
        {
            // cout << int(stream[i]) << " " ;
            tokenToReturn += stream[i];
            i++;
        }
        if (i < stream.size())
            stream = stream.substr(i);
        else
            stream = "";
        return tokenToReturn;
    }
    return "";
}

int convertToNumber(string s)
{
    if (isNumber(s))
    {
        return stoi(s);
    }
    else
    {
        if (addressOfLabel.find(s) != addressOfLabel.end())
            return addressOfLabel[s];
        else
        {
            cerr << "Label name is not defined: " + s;
            exit(1);
        }
    }
}

string numberToBinary(int num)
{
    bitset<3> binary(num);
    return binary.to_string();
}

string numberToBinaryI(int num)
{
    if (num < -32768 || num > 32767)
    {
        cerr << "offset argument overflow: " + to_string(num);
        exit(1);
    }
    bitset<16> binary(num);
    return binary.to_string();
}

string numberToBinary32(int num)
{
    bitset<32> binary(num);
    return binary.to_string();
}

int binaryStringToDem(string a)
{
    int res = 0;
    reverse(a.begin(), a.end());
    for (int i = 0; i < a.size(); i++)
    {
        if (a[i] == '1')
            res |= (1 << i);
    }
    return res;
}

bool validateLabel(string label)
{
    if (label.size() > 6)
    {
        cerr << "Label name must not exceed 6 characters: " + label ;
        exit(1);
    }
    if (addressOfLabel.find(label) != addressOfLabel.end())
    {
        cerr << "duplicated label name found: " + label;
        exit(1);
    }
    return true;
}

void initParser()
{
    operationType["add"] = "R";
    operationType["nand"] = "R";
    operationType["lw"] = "I";
    operationType["sw"] = "I";
    operationType["beq"] = "I";
    operationType["jalr"] = "J";
    operationType["noop"] = "O";
    operationType["halt"] = "O";

    opcode["add"] = "000";
    opcode["nand"] = "001";
    opcode["lw"] = "010";
    opcode["sw"] = "011";
    opcode["beq"] = "100";
    opcode["jalr"] = "101";
    opcode["noop"] = "111";
    opcode["halt"] = "110";
}
