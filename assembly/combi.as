    lw 0 1 n
    lw 0 2 r
    lw 0 6 comAdr
    lw 0 4 neg1 //$4 = -1
    jalr 6 7 //jump to combi
    halt
combi beq 0 2 base //check r == 0
    beq 1 2 base //check n == r
    lw 0 6 pos1
    sw 5 7 stack //remember caller
    add 5 6 5
    add 1 4 1 // n--
    sw 5 1 stack //save n - 1 to stack
    add 5 6 5
    sw 5 2 stack //save r to stack
    add 5 6 5 
    lw 0 6 comAdr //load address of combi
    jalr 6 7 // recursive n-1 r
    add 5 4 5
    lw 5 2 stack //load r from stack
    add 5 4 5
    lw 5 1 stack //load n from stack
    add 2 4 2 //r--
    lw 0 6 pos1
    sw 5 3 stack //save return value to stack
    add 5 6 5
    lw 0 6 comAdr 
    jalr 6 7 //recursive n-1 r-1
    add 5 4 5
    lw 5 6 stack //load n-1 r value from stack
    add 3 6 3
    beq 0 0 end
base lw 0 3 pos1 //return 1 if base case
    jalr 7 6 //jump back to caller
end add 5 4 5
    lw 5 7 stack // load caller address
    jalr 7 6 //jump back to caller
comAdr .fill combi //store combi address
pos1 .fill 1
neg1 .fill -1
n .fill 5 // store input n
r .fill 3 // store input r
stack .fill 0
