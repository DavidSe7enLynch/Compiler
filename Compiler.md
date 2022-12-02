### Compiler

- high-level re-compute prevention

- operating system/computer architecture course here

- imm val vs reg/memory

- how many global/local register allocation

- pushq popq for int?

- vr0 beginning of main?

- The register allocator will need to interact with the allocation of memory storage, to ensure that space is available to store spilled registers. For each basic block, compute the amount of spill storage needed, and the maximum over all basic blocks is the overall amount needed.?

- local value numbering: keep track of all temporary values? never reuse?

- order: lvn: 

  each number has its first operand and its calculation based on other numbers, each operand has a number; 

  and then use first operand of the number; 

  and finally remove duplicate def by living variable?



##### Assign5

- functional globally assign machine registers for (at least some) local variables, especially loop variables (callee saved regs, need to be push/pop)
- blockly locally allocate machine registers for virtual registers
- use local value numbering to eliminate redundant computation and then eliminate useless instructions
- Elimination of high-level instructions which assign a value to a virtual register whose value is not used subsequently (dead store elimination)



