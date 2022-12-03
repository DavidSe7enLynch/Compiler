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

class HighlevelLocalOptimize : public ControlFlowGraphTransform {
private:
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
    virtual std::shared_ptr<InstructionSequence> transform_basic_block(const BasicBlock *orig_bb);
    KeyMember getKeyMember(const Operand &operand);
    std::shared_ptr<LVNKey> getLVNKey(Instruction *ins);
    void getLiveVregs(const BasicBlock *orig_bb);
};


#endif //HW5_HIGHLEVEL_LOCAL_OPTIMIZE_H
