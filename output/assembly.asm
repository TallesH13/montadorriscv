srl x10 x22, x9
lw x9, 0(x10)
beq x9, x10, ELSE
xor x9, x10, x22
sub x9, x10, x9
ELSE: addi x10, x10, 4
sw x9, 0(x10)