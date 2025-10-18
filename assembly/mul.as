    lw 0 1 input1
    lw 0 2 input2
    lw 0 4 neg1
    lw 0 5 front1
    lw 0 6 pos1
cpi1 nand 5 1 7  //check if input1 is positive. If it is, in this function, $7 = 0; otherwise, $7 won't equal 0.
    nand 4 7 7
    beq 0 7 cand1  //check $7 == 0
cpi2 nand 5 2 7  //check if input1 is positive. If it is, in this function, $7 = 0; otherwise, $7 won't equal 0.
    nand 4 7 7
    beq 0 7 cand2  //check $7 == 0
bneg add 4 1 1 
    nand 4 1 1  //input1 = -input1
    add 4 2 2
    nand 4 2 2  //input2 = -input2
    beq 0 0 mul
cand1 beq 0 0 mul
cand2 add 0 1 7  //swap values, input1 = input2 and input2 = input1 (input1 is negetive). temp = input1  
    add 0 2 1  //input1 = input2
    add 0 7 2  //input2 = temp
    beq 0 0 mul
mul beq 1 0 end  //check input1 == 0
    nand 6 1 7 
    nand 7 7 7  //temp = input1 & pos1
    beq 7 6 func  //check temp == pos1
    beq 0 0 loop
func add 2 3 3  //res = res + input2
    add 4 1 1  //input1 = input1 - neg1
    beq 0 0 loop
loop add 4 4 4  //neg1 = neg1 + neg1
    add 6 6 6  //pos1 = pos1 + pos1
    add 2 2 2  //input2 = input2 + input2
    beq 0 0 mul
end halt
pos1   .fill 1
neg1    .fill -1
front1  .fill -2147483648
input1  .fill 231
input2  .fill -465
