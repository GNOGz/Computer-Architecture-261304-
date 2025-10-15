#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <bits/stdc++.h>

using namespace std;
map<string, string> operationType;
map<string, string> opcode;
map<string, int> addressOfLabel;

string stream; // global variable to store each operation in each line

// R-type instructions (add, nand) มี 3 fields (field0 คือ regA, field1 คือ regB, field2 คือ destReg)
// I-type instructions (lw, sw, beq) มี 3 fields (field0 คือ regA, field1 คือ regB, field2 เป็น ค่าตัวเลขสำหรับ offsetField ซึ่งเป็นได้ทั้ง บวกหรือลบ หรือ symbolic address ซึ่งจะกล่าวถึงข้างล่าง
// J-type instructions (jalr) มี 2 fields (field0 คือ regA, field1 คือ regB)
// O-type instruction (noop, halt) ไม่มี field

// int readAndParse(FILE *, string, string, string, string, string);

bool isNumber(string);

long long binaryToDecimal(vector<string>);
void initParser();
vector<string> readAllLines(string);
string tokenize();
vector<string> numTokenize(string, int);
int convertToNumber(string);
string numberToBinary(int);
string numberToBinaryI(int);
string numberToBinary32(int);
int binaryStringToDem(string);

int main(int argc, char *argv[])
{
    // if (argc != 3)
    // {
    //     printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
    //            argv[0]);
    //     exit(1);
    // }

    vector<string> line = readAllLines("1.txt");
    freopen("output.txt", "w", stdout);
    vector<string> binaryResult;
    initParser();
    for (int i = 0; i < line.size(); i++)
    { // first loop is to loop through all lines and save the address of the label.
        // check whitespace to see if it has label or not
        stream = line[i];
        // cout << line[i] << endl;
        if (stream[i] != ' ' && stream[i] != '\0' && stream[i] != '\t')
        { // label found since it is not white space in the start
            string labelName = tokenize();
            addressOfLabel[labelName] = i; // save label name to map
            continue;
        }
        string labelName = tokenize();
        string check = tokenize();
        if (check == ".fill")
        {
            string addressValue = tokenize();
            int val;
            if (isNumber(addressValue))
            {
                val = stol(addressValue);
            } // if .fill and then immediate, we can save the value right away. :)
            else
            {
                val = addressOfLabel[addressValue];
            } // if it's the label. we need to look up the value in the map.
            addressOfLabel[labelName] = val; // save the value to map.
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
        binary += opcode[operation];
        // cout << operation << endl;
        if (operationType[operation] == "R")
        { // 3 fields
            regA = tokenize();
            regB = tokenize();
            regDes = tokenize();
            valRegA = numberToBinary(convertToNumber(regA));
            valRegB = numberToBinary(convertToNumber(regB));
            regDes = numberToBinary(convertToNumber(regDes));
            binary += valRegA + valRegB + "0000000000000" + regDes;
        }
        else if (operationType[operation] == "I")
        { // 3 fields
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
                ValOffest = numberToBinaryI(convertToNumber(offset));
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
            continue;
        }
        else
        {
            // cout <<"code: " << line[i] << "|| operation: " << operation << endl;
            throw runtime_error("Unexpected operation"+ operation + "at line " + to_string(i));
        }
        binaryResult.push_back(binary);
    }

    for (int i = 0; i < binaryResult.size(); i++)
    {
        // cout<<"addresss " << i <<" " <<binaryResult[i]  << "|| size : " << binaryResult[i].size() << endl;
        // cout <<"address " << i << ": " << binaryStringToDem(binaryResult[i])<< " || "<<line[i] << endl;
        cout << binaryStringToDem(binaryResult[i]) << endl;
    }
}

vector<string> readAllLines(string path)
{
    ifstream file(path); // open your file
    if (!file.is_open())
    {
        throw runtime_error("file can not  open file");
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
        return stoi(s);
    else
    {
        return addressOfLabel[s];
    }
}

string numberToBinary(int num)
{
    bitset<3> binary(num);
    return binary.to_string();
}

string numberToBinaryI(int num)
{
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
    opcode["nadd"] = "001";
    opcode["lw"] = "010";
    opcode["sw"] = "011";
    opcode["beq"] = "100";
    opcode["jalr"] = "101";
    opcode["noop"] = "111";
    opcode["halt"] = "110";
}
