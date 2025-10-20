	lw 0 1 input1 //load input1 into register1
	lw 0 2 input2 //load input2 into register2
gcd	lw 0 6 one
	lw 0 4 neg1
	nand 4 2 4 // flip bits of value in reg2 and store it in reg4
	add 4 6 4 // reg4 = reg2 + 1 result in $4 = -$2 
	add 1 4 4 // $4 = $4 - $1 result in $4 = $1 - $2
	lw 0 6 front1
	nand 4 6 4
	nand 4 4 4 // this and above instruction equal to $4 & $6 this result in checking wether $4 is negative
	beq 0 4 check // if $4 is not negative mean $1 is greater or equal to $2 if so skip swap value
swap lw 0 6 one 
	sw 5 1 stack // store $1 in stack 
	add 5 6 5
	sw 5 2 stack // store $2 in stack
	lw 5 1 stack // load $2 from stack to $1
	lw 0 6 neg1
	add 5 6 5 
	lw 5 2 stack // load $1 from stack to $2 
check beq 0 2 end // if $2 equal 0 then answer is in $1 
	lw 0 6 one
	lw 0 4 neg1
	nand 4 2 4 // flip bits of value in reg2 and store it in reg4
	add 4 6 4 // reg4 = reg2 + 1 result in $4 = -$2 
	add 1 4 1 // $1 = $4 - $1 result in $1 = $1 - $2
	lw 0 4 prog 
	jalr 4 7 // recursive call gcd(a - b, b)
end	halt
prog .fill gcd	
input1	.fill 31250
input2	.fill 118098
one .fill 1
neg1 .fill -1
front1 .fill -2147483648 //this is 0b100000000000000000000000000000000
stack	.fill 0
