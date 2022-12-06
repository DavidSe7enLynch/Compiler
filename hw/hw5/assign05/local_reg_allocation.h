//
// Created by Ruirong Huang on 12/4/22.
//

#ifndef HW5_LOCAL_REG_ALLOCATION_H
#define HW5_LOCAL_REG_ALLOCATION_H

#include <map>
#include <memory>
#include <cassert>
#include "node.h"
#include "cfg_transform.h"
#include "live_vregs.h"
#include "highlevel_defuse.h"
#include "print_highlevel_code.h"
#include "highlevel_formatter.h"
#include "lowlevel.h"
#include "highlevel.h"
#include "lowlevel_formatter.h"

/**
    memory:

    1. func vars that must be in memory (not a vreg): array/struct
    2. (vregs that are alive between blocks - func vars) + func vars cannot be stored in callee-saved mregs
    3. spilled temp vregs

    1 is already calculated local_storage_allocation
    2 need to be determined for the whole cfg
    3 need to be determined for each block
    each vreg takes 8 bytes
    2: (alive at beg/end of blocks && !func vars) || !mreg_allocated func vars

 */
class LocalRegAllocation : public ControlFlowGraphTransform {
private:
    // for whole cfg, will not change once initialized
    int m_localStorageClass1;
    LiveVregs m_liveVregAll;
    LiveVregs::FactType m_funcVars; // calculated during construction
    LiveVregs::FactType m_mregFuncVars; // calculated during construction
    LiveVregs::FactType m_memVreg; // class 2, calculated by calMemVreg()
    // memory allocation for class 2 vregs
    // <vreg, mem addr>
    std::map<int, long> m_mapVregMemClass2; // calculated by allocateMemory()
    // space for spill
    std::vector<int> m_spillAddr; // calculated by allocateMemory()
    long m_totalMemory;

    // for each block
    LiveVregs::FactType m_factBeg;
    LiveVregs::FactType m_factEnd;
    // vr1-vr6
    // %rdi, %rsi, %rdx, %rcx, %r8, %r9
    std::vector<MachineReg> m_availMregs;
    // 2 maps
    // <mreg, vreg>
    // <vreg, <mreg, size>>
    std::map<MachineReg, int> m_mapMregVreg;
    std::map<int, std::pair<MachineReg, int>> m_mapVregMreg;
    // 2 maps for spilled mregs
    // spilled mreg map, always spill 8 byte mreg
    // <spilled mreg, vreg>
    // <vreg, <mreg, size>>
    std::map<MachineReg, int> m_mapSpilledMregVreg;
    std::map<int, std::pair<MachineReg, int>> m_mapSpilledVregMreg;

public:
    LocalRegAllocation(const std::shared_ptr<ControlFlowGraph> &cfg, const std::shared_ptr <InstructionSequence> &hl_iseq);

    // do storage allocation here
    virtual std::shared_ptr<ControlFlowGraph> transform_cfg();

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
    void printMapVec();
    void calMemVreg();

    /**
     * allocate memory for this function
     *
     * memory:
     * 1. func vars that must be in memory (not a vreg): array/struct
     * 2. (vregs that are alive between blocks - func vars) + func vars cannot be stored in callee-saved mregs
     * 3. spilled temp vregs
     *
     * 1: m_localStorageClass1
     * 2: m_memVreg
     * 2: (alive at beg/end of blocks && !func vars) || !mreg_allocated func vars
     * 3: reserve max place needed () and allocate later
     */
    void allocateMemory();
    int calMaxSpill();
};


#endif //HW5_LOCAL_REG_ALLOCATION_H
