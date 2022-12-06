This HW5 is about optimization of generated codes

1. global register allocation
I added a map to store <int, num of occur>
I used a ratio for for/while loops
use r12-r15 callee saved registers to do this

if more than 4 variables, store in memory

2. LVN

3. local register allocation

4. lowlevel code generation optimization


memory:

1. func vars that must be in memory (not a vreg): array/struct
2. vregs that are alive between blocks and cannot be stored in callee-saved mregs
3. spilled temp vregs

1 is already calculated local_storage_allocation
so we need to deal with 23
each vreg takes 8 bytes

