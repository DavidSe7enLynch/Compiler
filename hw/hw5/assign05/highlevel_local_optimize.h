//
// Created by Ruirong Huang on 12/2/22.
//

#ifndef HW5_HIGHLEVEL_LOCAL_OPTIMIZE_H
#define HW5_HIGHLEVEL_LOCAL_OPTIMIZE_H

#include <map>
#include <memory>
#include "cfg_transform.h"
#include "local_value_numbering.h"
#include "live_vregs.h"
#include "highlevel_defuse.h"
#include "print_highlevel_code.h"

class HighlevelLocalOptimize : public ControlFlowGraphTransform {
private:
    bool m_isPrint = false;
    // map 1: <vreg: int, valnum class>
    // map 2: <lvnKey class, valnum class>
    // beg fact
    // end fact
    // liveVreg
    std::map <int, std::shared_ptr<ValueNumber>> m_mapVregVal;
    std::map <std::shared_ptr<LVNKey>, std::shared_ptr<ValueNumber>> m_mapKeyVal;
    LiveVregs::FactType m_factBeg;
    LiveVregs::FactType m_factEnd;
    LiveVregs m_liveVregAll;
public:
    HighlevelLocalOptimize(const std::shared_ptr<ControlFlowGraph> &cfg);
    // Create a transformed version of the instructions in a basic block.
    // Note that an InstructionSequence "owns" the Instruction objects it contains,
    // and is responsible for deleting them. Therefore, be careful to avoid
    // having two InstructionSequences contain pointers to the same Instruction.
    // If you need to make an exact copy of an Instruction object, you can
    // do so using the duplicate() member function, as follows:
    //
    //    Instruction *orig_ins = /* an Instruction object */
    //    Instruction *dup_ins = orig_ins->duplicate();
    virtual std::shared_ptr<InstructionSequence> transform_basic_block(const BasicBlock *orig_bb);
private:
    KeyMember getKeyMember(const Operand &operand);
    std::shared_ptr<LVNKey> getLVNKey(Instruction *ins);
    void localValueNumbering(const BasicBlock *orig_bb);
    void printMaps();
    bool isDuplicate(int vreg);
    bool isConstNum(int vreg);
    long getConstNum(int vreg);
    bool hasValNum(int vreg);
    bool isArgRetVreg(int vreg);
};


#endif //HW5_HIGHLEVEL_LOCAL_OPTIMIZE_H
