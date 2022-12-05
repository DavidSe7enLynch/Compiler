//
// Created by Ruirong Huang on 12/2/22.
//

#include "highlevel_local_optimize.h"

HighlevelLocalOptimize::HighlevelLocalOptimize(const std::shared_ptr <ControlFlowGraph> &cfg)
        : ControlFlowGraphTransform(cfg), m_liveVregAll(cfg) {
    m_liveVregAll.execute();
}

std::shared_ptr <InstructionSequence> HighlevelLocalOptimize::transform_basic_block(
        const BasicBlock *orig_bb) {
//    std::printf("in block %d ==================\n", orig_bb->get_id());
    if (orig_bb->get_kind() != BASICBLOCK_INTERIOR) {
        // ENTRY or EXIT block
        std::shared_ptr <InstructionSequence> new_bb(new InstructionSequence());
        return new_bb;
    }

    // normal block
    // steps:
    // LVN
    // get new bb by eliminating useless def

    localValueNumbering(orig_bb);

    std::shared_ptr <InstructionSequence> new_bb(new InstructionSequence());
    for (auto i = orig_bb->cbegin(); i != orig_bb->cend(); ++i) {
        // in some cases, just copy
        // no need to deal with numOperand = 0 or 1: enter, call, etc.
        if ((*i)->get_num_operands() <= 1) {
            new_bb->append((*i)->duplicate());
            continue;
        }
        // replace all use of duplicate vregs to first vregs
        // remove def ins of useless vregs
        const Operand &opDst = (*i)->get_operand(0);
        if (opDst.get_kind() == Operand::VREG
                && (isDuplicate(opDst.get_base_reg()) || isConstNum(opDst.get_base_reg()))) {
            // is a def, and is duplicate or compile-time const
            // skip this ins
//            std::printf("delete ins: %s\n", HighLevelFormatter().format_instruction(*i).c_str());
            continue;
        }

        Instruction *newIns = (*i)->duplicate();
        for (int j = 0; j < (*i)->get_num_operands(); j++) {
            Operand operand = (*i)->get_operand(j);
            if (HighLevel::is_use(*i, j)) {
                int vreg = operand.get_base_reg();
                // use of a duplicate or compile-time const reg
                // replace it with first reg or const
                if (isDuplicate(vreg) && !isConstNum(vreg)) {
                    newIns->setOperand(j, Operand(operand.get_kind(), m_mapVregVal.at(vreg)->getOrigVreg()));
                } else if (isConstNum(vreg)) {
                    newIns->setOperand(j, Operand(Operand::IMM_IVAL, m_mapVregVal.at(vreg)->getConstNum()));
                }
            }
        }
        new_bb->append(newIns);
    }

    // clear all global maps/vectors
    m_mapVregVal.clear();
    m_mapKeyVal.clear();
    m_factBeg.reset();
    m_factEnd.reset();

//    std::printf("\n\nstart printing new_bb %d ============\n\n", orig_bb->get_id());
//    PrintHighLevelCode().print_instructions(new_bb);
//    std::printf("\n\nend printing new_bb %d ============\n\n", orig_bb->get_id());
    return new_bb;
}

KeyMember HighlevelLocalOptimize::getKeyMember(const Operand &operand) {
    std::map < int, std::shared_ptr < ValueNumber >> ::iterator iter;
    if (operand.get_kind() == Operand::VREG &&
        (iter = m_mapVregVal.find(operand.get_base_reg())) != m_mapVregVal.end()) {
        // if src operand has a value number
        // use value number
        return KeyMember(iter->second);
    } else {
        // use operand, including all non-vreg operand, and those don't have value number
        return KeyMember(operand);
    }
}

std::shared_ptr <LVNKey> HighlevelLocalOptimize::getLVNKey(Instruction *ins) {
    int numOperand = ins->get_num_operands();
    assert(numOperand >= 2);
    const Operand &opSrcL = ins->get_operand(1);

    if (numOperand == 2) {
        std::shared_ptr <LVNKey> lvnKey(new LVNKey(ins->get_opcode(), getKeyMember(opSrcL)));
        return lvnKey;
    } else { // numOperand = 3
        const Operand &opSrcR = ins->get_operand(2);
        std::shared_ptr <LVNKey> lvnKey(new LVNKey(ins->get_opcode(), getKeyMember(opSrcL), getKeyMember(opSrcR)));
        return lvnKey;
    }
}

void HighlevelLocalOptimize::localValueNumbering(const BasicBlock *orig_bb) {
    // get live vregs
    int valNum = 0;
    m_factBeg = m_liveVregAll.get_fact_at_beginning_of_block(orig_bb);
    m_factEnd = m_liveVregAll.get_fact_at_end_of_block(orig_bb);

    // scan instructions, label valNum for dst

    for (auto i = orig_bb->cbegin(); i != orig_bb->cend(); ++i) {
        // steps:
        // create LVNKey
        // create ValueNumber
        // store them into 2 maps

        // no need to deal with numOperand = 0 or 1: enter, call, etc.
        // no need to deal with non-reg dst
        // avoid handling dst in end-alive or start-alive
        // avoid handling dst for call args (vr1-vr9)
        if ((*i)->get_num_operands() <= 1
                || (*i)->get_operand(0).get_kind() != Operand::VREG
                || isArgVreg((*i)->get_operand(0).get_base_reg())
                || m_factBeg.test((*i)->get_operand(0).get_base_reg())
                || m_factEnd.test((*i)->get_operand(0).get_base_reg())) {
//            std::printf("%s.........\n\n", HighLevelFormatter().format_instruction(*i).c_str());
            continue;
        }

        // get LVNKey for dst vreg
        std::shared_ptr <LVNKey> lvnKey = getLVNKey((*i));
//        std::printf("%s.........%s\n", HighLevelFormatter().format_instruction(*i).c_str(), lvnKey->toStr().c_str());

        // check if lvnKey existed
        const Operand &opDst = (*i)->get_operand(0);
        std::map < std::shared_ptr < LVNKey > , std::shared_ptr < ValueNumber >>::iterator iter;
        for (iter = m_mapKeyVal.begin(); iter != m_mapKeyVal.end(); ++iter) {
            if (iter->first->isSame(lvnKey)) {
                // lvnKey existed
                // label dst vreg with orig valNum
                m_mapVregVal.insert(std::pair < int, std::shared_ptr < ValueNumber >> (opDst.get_base_reg(), iter->second));
//                std::printf("val existed: %s\n\n", iter->second->toStr().c_str());
                break;
            }
        }

        if (iter == m_mapKeyVal.end()) {
            // not exist
            // add new key and new value to 2 maps
            std::shared_ptr <ValueNumber> value(new ValueNumber(valNum++, opDst.get_base_reg(), lvnKey));
            m_mapVregVal.insert(std::pair < int, std::shared_ptr < ValueNumber >> (opDst.get_base_reg(), value));
            m_mapKeyVal.insert(std::pair < std::shared_ptr < LVNKey > ,
                               std::shared_ptr < ValueNumber >> (lvnKey, value));
//            std::printf("val new: %s\n\n", value->toStr().c_str());
        }
    }
//    printMaps();
}

void HighlevelLocalOptimize::printMaps() {
    // print 2 maps
    std::map < int , std::shared_ptr < ValueNumber >>::iterator iterVregVal;
    std::map < std::shared_ptr < LVNKey > , std::shared_ptr < ValueNumber >>::iterator iterKeyVal;

    std::printf("\n\n\nmapVregVal===========\n\n");
    for (iterVregVal = m_mapVregVal.begin(); iterVregVal != m_mapVregVal.end(); ++iterVregVal) {
        std::printf("(%d, %s)\n", iterVregVal->first, iterVregVal->second->toStr().c_str());
    }
    std::printf("\n\n\nmapKeyVal===========\n\n");
    for (iterKeyVal = m_mapKeyVal.begin(); iterKeyVal != m_mapKeyVal.end(); ++iterKeyVal) {
        std::printf("(%s, %s)\n", iterKeyVal->first->toStr().c_str(), iterKeyVal->second->toStr().c_str());
    }
}

bool HighlevelLocalOptimize::isDuplicate(int vreg) {
    if (!hasValNum(vreg)) {
        return false;
    }
    int origVreg = m_mapVregVal.at(vreg)->getOrigVreg();
    return origVreg != vreg;
}

bool HighlevelLocalOptimize::isConstNum(int vreg) {
    if (!hasValNum(vreg)) {
        return false;
    }
    return m_mapVregVal.at(vreg)->isConstNum();
}

bool HighlevelLocalOptimize::hasValNum(int vreg) {
    return (m_mapVregVal.count(vreg) > 0);
}

long HighlevelLocalOptimize::getConstNum(int vreg) {
    assert(isConstNum(vreg));
    return m_mapVregVal.at(vreg)->getConstNum();
}

bool HighlevelLocalOptimize::isArgVreg(int vreg) {
    return (vreg >= 1 && vreg <= 9);
}

