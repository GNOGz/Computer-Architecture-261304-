	lw 0 1 input1
	lw 0 2 input2
gcd	lw 0 6 one
	lw 0 4 all1
	nand 4 2 4
	add 4 6 4
	add 1 4 4
	lw 0 6 front1
	nand 4 6 4 
	nand 4 4 4
	beq  0 4 check 
swap 	lw 0 6 one 
	sw 5 1 stack 
	add 5 6 5
	sw 5 2 stack 
	lw 5 1 stack
	lw 0 6 neg1
	add 5 6 5
	lw 5 2 stack 
check	beq 0 2 end
	lw 0 6 one
	lw 0 4 all1
	nand 4 2 4
	add 4 6 4
	add 1 4 1
	lw 0 4 prog
	jalr 4 7
end	sw 5 1 stack
	halt
prog .fill gcd	
input1	.fill 33
input2	.fill 63
one .fill 1
neg1 .fill -1
front1 .fill -2147483648
all1 .fill -1
stack	.fill 0
