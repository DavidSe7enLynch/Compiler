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



issues met and solved:

over the size of int when doing compile-time const number calculation example22

imm_label, label example16

negative memory example1

reassign (a compile-time const num) to an existed func var example22

consider "count + 1" as the same key

forget to use the bottom as memoffset: 0 or -8 example28

why my LVN could be correct, each redefine of a funcvar will be assigned to a new valnum, which makes the key
different, so none will be eliminated

No use of funcvar??might be the problem of example29
