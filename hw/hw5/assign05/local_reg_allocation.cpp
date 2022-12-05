//
// Created by Ruirong Huang on 12/4/22.
//

#include "local_reg_allocation.h"

LocalRegAllocation::LocalRegAllocation(const std::shared_ptr <ControlFlowGraph> &cfg, const std::shared_ptr <InstructionSequence> &hl_iseq)
        : ControlFlowGraphTransform(cfg), m_liveVregAll(cfg) {
    m_liveVregAll.execute();
    for (auto i : hl_iseq->getCalleeVec()) {
        m_funcVars.set(i.first);
    }
}

std::shared_ptr <InstructionSequence> LocalRegAllocation::transform_basic_block(const BasicBlock *orig_bb) {
    // init
    if (orig_bb->get_kind() != BASICBLOCK_INTERIOR || orig_bb->get_length() == 0) {
        // ENTRY or EXIT block
        std::shared_ptr <InstructionSequence> new_bb(new InstructionSequence());
        return new_bb;
    }
    m_factBeg = m_liveVregAll.get_fact_at_beginning_of_block(orig_bb);
    m_factEnd = m_liveVregAll.get_fact_at_end_of_block(orig_bb);
    initAvailMregs(orig_bb);
    m_mapMregVreg.clear();
    std::printf("start block %d============\n", orig_bb->get_id());
    printMapVec();

    // new_bb
    std::shared_ptr <InstructionSequence> new_bb(new InstructionSequence());

    // no need to deal with beg/end alive vregs
    // no need to deal with function variables
    // do not consider vr0-vr9
    for (auto i = orig_bb->cbegin(); i != orig_bb->cend(); ++i) {
        // clear dead vreg allocation
        clearDeadAlloc(orig_bb, *i);

        Instruction *newIns = (*i)->duplicate();

        // find temp vreg
        for (auto j = 0; j < (*i)->get_num_operands(); j++) {
            const Operand &operand = (*i)->get_operand(j);
            if (operand.has_base_reg()) {
                int vreg = operand.get_base_reg();
                if (!m_factBeg.test(vreg)
                        && !m_factEnd.test(vreg)
                        && !m_funcVars.test(vreg)
                        && vreg > 9
                        && !isAllocatedVreg(vreg)) {
                    // get a new temp vreg, store it in one avail mreg
                    if (m_availMregs.empty()) {
                        // no available mreg
                        // TODO: spill and restore
                        std::printf("\n\n\n\n============Warning, no available mreg=============\n\n\n\n");
                    } else {
                        MachineReg mreg = m_availMregs.back();
                        // delete from avail
                        m_availMregs.pop_back();
                        // add pair to map
                        m_mapMregVreg.insert(std::pair<MachineReg, int>(mreg, vreg));
                        // set to new operand
                        int size = highlevel_opcode_get_idx_operand_size(HighLevelOpcode((*i)->get_opcode()), j);
                        std::pair<MachineReg, int> mregPair(mreg, size);
                        Operand newOperand(operand.get_kind(), operand.get_base_reg());
                        newOperand.setMreg(mregPair);
                        newIns->setOperand(j, newOperand);
                        std::printf("vreg %d mapped to mreg %d, mregPair<%d, %d>\n", vreg, (int)mreg, (int)mreg, size);
                    }
                }
            }
        }
        new_bb->append(newIns);
        printMapVec();
    }

    std::printf("\n\nstart printing block %d============\n\n", orig_bb->get_id());
    PrintHighLevelCode().print_instructions(new_bb);
    std::printf("\n\nend printing block %d============\n\n", orig_bb->get_id());
    return new_bb;
}

void LocalRegAllocation::initAvailMregs(const BasicBlock *orig_bb) {
    // vr1-vr6
    // %rdi, %rsi, %rdx, %rcx, %r8, %r9
    if (orig_bb->get_last_instruction()->get_opcode() != HINS_call) {
        // no func call
        setAvailMregs(6);
        return;
    }
    // has func call
    int numArgs = 0;
    for (auto i = orig_bb->cbegin(); i != orig_bb->cend(); ++i) {
        if ((*i)->get_num_operands() > 0
                && (*i)->get_operand(0).get_kind() == Operand::VREG
                && isArgVreg((*i)->get_operand(0).get_base_reg())) {
            // is def of vr1-vr9
            numArgs += 1;
        }
    }
    setAvailMregs(6 - numArgs);
}

void LocalRegAllocation::setAvailMregs(int numAvail) {
    m_availMregs.clear();
    MachineReg availMregsArray[6] = {MREG_RDI, MREG_RSI, MREG_RDX, MREG_RCX, MREG_R8, MREG_R9};
    for (auto i = 0; i < numAvail; i++) {
        m_availMregs.push_back(availMregsArray[6 - numAvail + i]);
    }
}

bool LocalRegAllocation::isArgVreg(int vreg) {
    return (vreg >= 1 && vreg <= 9);
}

void LocalRegAllocation::clearDeadAlloc(const BasicBlock *orig_bb, Instruction *ins) {
    LiveVregs::FactType factInsBef = m_liveVregAll.get_fact_before_instruction(orig_bb, ins);
    LiveVregs::FactType factInsAft = m_liveVregAll.get_fact_after_instruction(orig_bb, ins);

    std::printf("\n%s\n", m_liveVregAll.fact_to_string(factInsBef).c_str());
    std::printf("%s\n", HighLevelFormatter().format_instruction(ins).c_str());
    std::printf("%s\n", m_liveVregAll.fact_to_string(factInsAft).c_str());

    for (auto iter = m_mapMregVreg.begin(); iter != m_mapMregVreg.end();) {
        if (!factInsBef.test(iter->second)) {
            // dead vreg
            std::printf("vreg %d died, mreg %d released\n", iter->second, (int)iter->first);
            m_availMregs.push_back(iter->first);
            m_mapMregVreg.erase(iter++);
        } else {
            iter++;
        }
    }
}

int LocalRegAllocation::highlevel_opcode_get_idx_operand_size(HighLevelOpcode opcode, int idx) {
    if (idx == 0) {
        return highlevel_opcode_get_dest_operand_size(opcode);
    }
    return highlevel_opcode_get_source_operand_size(opcode);
}

bool LocalRegAllocation::isAllocatedVreg(int vreg) {
    for (auto iter = m_mapMregVreg.begin(); iter != m_mapMregVreg.end(); ++iter) {
        if (iter->second == vreg) return true;
    }
    return false;
}

void LocalRegAllocation::printMapVec() {
    // print map and vector
    std::printf("\nmapMregVreg===========\n");
    for (auto iter = m_mapMregVreg.begin(); iter != m_mapMregVreg.end(); ++iter) {
        std::printf("(%d, %d), ", (int)iter->first, iter->second);
    }
    std::printf("\nvecAvailMreg===========\n");
    for (auto iter = m_availMregs.begin(); iter != m_availMregs.end(); ++iter) {
        std::printf("%d, ", (int)*iter);
    }
    std::printf("\n\n");
}
