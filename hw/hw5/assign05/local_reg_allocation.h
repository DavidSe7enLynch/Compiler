//
// Created by Ruirong Huang on 12/4/22.
//

#ifndef HW5_LOCAL_REG_ALLOCATION_H
#define HW5_LOCAL_REG_ALLOCATION_H

#include <map>
#include <memory>
#include "cfg_transform.h"
#include "live_vregs.h"
#include "highlevel_defuse.h"
#include "print_highlevel_code.h"
#include "highlevel_formatter.h"
#include "lowlevel.h"
#include "highlevel.h"
#include "lowlevel_formatter.h"


class LocalRegAllocation : public ControlFlowGraphTransform {
private:
    LiveVregs m_liveVregAll;
    LiveVregs::FactType m_factBeg;
    LiveVregs::FactType m_factEnd;
    LiveVregs::FactType m_funcVars;
    // vr1-vr6
    // %rdi, %rsi, %rdx, %rcx, %r8, %r9
    std::vector<MachineReg> m_availMregs;
    std::map<MachineReg, int> m_mapMregVreg;
public:
    LocalRegAllocation(const std::shared_ptr<ControlFlowGraph> &cfg, const std::shared_ptr <InstructionSequence> &hl_iseq);

    // Create a transformed version of the instructions in a basic block.
    // Note that an InstructionSequence "owns" the Instruction objects it contains,
    // and is responsible for deleting them. Therefore, be careful to avoid
    // having two InstructionSequences contain pointers to the same Instruction.
    // If you need to make an exact copy of an Instruction object, you can
    // do so using the duplicate() member function, as follows:
    //
    //    Instruction *orig_ins = /* an Instruction object */
    //    Instruction *dup_ins = orig_ins->duplicate();

    /**
     * do local register allocation on high-level optimized code
     *
     */
    virtual std::shared_ptr<InstructionSequence> transform_basic_block(const BasicBlock *orig_bb);
private:
    // if there is function call, how many args are used
    // the rest are available
    void initAvailMregs(const BasicBlock *orig_bb);
    void setAvailMregs(int numAvail);
    bool isArgVreg(int vreg);
    void clearDeadAlloc(const BasicBlock *orig_bb, Instruction *ins);
    int highlevel_opcode_get_idx_operand_size(HighLevelOpcode opcode, int idx);
    bool isAllocatedVreg(int vreg);
    void printMapVec();
};


#endif //HW5_LOCAL_REG_ALLOCATION_H
