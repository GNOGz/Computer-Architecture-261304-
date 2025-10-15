#include<bits/stdc++.h>
using namespace std;

string stream = "";

string tokenize(){
    string tokenToReturn = "";
    int i = 0;
    while(i < stream.size()){
        while(stream[i] == ' '){
            i++;
        }
        while(stream[i] != ' '){
            tokenToReturn += stream[i];
            i++;
        }
        stream = stream.substr(i);
        return tokenToReturn;
    }
    return " ";
    
}

int main(){
    stream = "lw 0 1 five  load reg1 with 5 (uses symbolic address)";
    for(int i = 0 ; i < 3;i++){
        cout << "tokenized word: " << tokenize() << " || word leff: " << stream << endl;
    }
}