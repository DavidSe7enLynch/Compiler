#ifndef LOWLEVEL_CODEGEN_H
#define LOWLEVEL_CODEGEN_H

#include <memory>
#include <map>
#include <algorithm>
#include "lowlevel.h"
#include "instruction_seq.h"

// A LowLevelCodeGen object transforms an InstructionSequence containing
// high-level instructions into an InstructionSequence containing
// low-level (x86-64) instructions.
class LowLevelCodeGen {
private:
    int m_total_memory_storage;
    int m_localStorage;
    bool m_optimize;

    // 0: 10, 1: 11, 2: rax
//    int numTempReg;
    bool m_ifUseRAX;

    // save callee-saved registers
    int m_numPush;
    MachineReg m_callee[5] = {MREG_R12, MREG_R13, MREG_R14, MREG_R15, MREG_RBX};
    std::map<int, MachineReg> m_mregMap;

    std::string m_returnLabel;

public:
    LowLevelCodeGen(bool optimize);

    virtual ~LowLevelCodeGen();

    std::shared_ptr <InstructionSequence> generate(const std::shared_ptr <InstructionSequence> &hl_iseq);

private:
    std::shared_ptr <InstructionSequence> translate_hl_to_ll(const std::shared_ptr <InstructionSequence> &hl_iseq);

    void translate_instruction(Instruction *hl_ins, const std::shared_ptr <InstructionSequence> &ll_iseq);

    Operand get_ll_operand(Operand hl_opcode, int size, const std::shared_ptr <InstructionSequence> &ll_iseq);

//    Operand nextTempOperand();
};

#endif // LOWLEVEL_CODEGEN_H
