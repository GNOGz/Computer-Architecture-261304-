    lw 0 2 mcand
    lw 0 3 mplier
    lw 0 4 neg1
    lw 0 5 front1
    lw 0 6 pos1
cpmc nand 5 2 7 //check if mcand is positive. If it is, in this function, $7 = 0; otherwise, $7 won't equal 0.
    nand 4 7 7
    beq 0 7 cand2 //check $7 == 0
cpmp nand 5 3 7 //check if mplier is positive. If it is, in this function, $7 = 0; otherwise, $7 won't equal 0.
    nand 4 7 7
    beq 0 7 cand3 //check $7 == 0
bneg add 4 2 2 
    nand 4 2 2 //mcand = -mcand
    add 4 3 3
    nand 4 3 3 //mplier = -mplier
    beq 0 0 mul
cand2 beq 0 0 mul
cand3 add 0 2 7 //swap values, mcand = mplier and mplier = mcand (input1 is negetive). temp = mcand
    add 0 3 2 //mcand = mplier
    add 0 7 3 //mplier = temp
    beq 0 0 mul
mul beq 2 0 end //check mcand == 0
    nand 6 2 7 
    nand 7 7 7 //temp = mcand & pos1
    beq 7 6 func //check temp == pos1
    beq 0 0 loop
func add 3 1 1 //res = res + mplier
    add 4 2 2 //mcand = mcand - neg1
    beq 0 0 loop
loop add 4 4 4 //neg1 = neg1 + neg1
    add 6 6 6 //pos1 = pos1 + pos1
    add 3 3 3 //mplier = mplier + mplier
    beq 0 0 mul
end halt
mcand  .fill 32766
mplier  .fill 10383
neg1    .fill -1
front1  .fill -2147483648
pos1   .fill 1
