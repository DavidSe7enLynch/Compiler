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
    // only deal with normal blocks with instructions
    // get live regs at beg/end
    // get available mregs
    // clear maps
    // get spilled num
    if (orig_bb->get_kind() != BASICBLOCK_INTERIOR || orig_bb->get_length() == 0) {
        // ENTRY or EXIT block
        std::shared_ptr <InstructionSequence> new_bb(new InstructionSequence());
        return new_bb;
    }
    m_factBeg = m_liveVregAll.get_fact_at_beginning_of_block(orig_bb);
    m_factEnd = m_liveVregAll.get_fact_at_end_of_block(orig_bb);
    initAvailMregs(orig_bb);
    m_mapMregVreg.clear();
    m_mapVregMreg.clear();
    calSpillAddr(orig_bb);

//    std::printf("start block %d============\n", orig_bb->get_id());
//    printMapVec();

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
            if (!operand.has_base_reg()) continue;

            int vreg = operand.get_base_reg();
            if (!m_factBeg.test(vreg)
                    && !m_factEnd.test(vreg)
                    && !m_funcVars.test(vreg)
                    && vreg > 9) {
                // get a temp vreg, store it in one avail mreg
                std::map<int, std::pair<MachineReg, int>>::iterator iter = m_mapVregMreg.find(vreg);
                if (iter != m_mapVregMreg.end()) {
                    // get an allocated vreg
                    // set a new operand with corresponding mreg
//                    MachineReg mreg = iter->second.first;
//                    int size = iter->second.second;
//                    std::pair<MachineReg, int> mregPair(mreg, size);
                    std::pair<MachineReg, int> mregPair(iter->second);
                    Operand newOperand(operand.get_kind(), operand.get_base_reg());
                    newOperand.setMreg(mregPair);
                    newIns->setOperand(j, newOperand);
                } else if (m_availMregs.empty()) {
                    // no available mreg
                    // TODO: spill and restore
                    std::printf("\n\n\n\n============Warning, no available mreg=============\n\n\n\n");
                } else {
                    // get a new temp vreg
                    MachineReg mreg = m_availMregs.back();
                    // delete from avail
                    m_availMregs.pop_back();

                    HighLevelOpcode opcode = HighLevelOpcode((*i)->get_opcode());
                    int size = highlevel_opcode_get_idx_operand_size(opcode, j);
                    std::pair<MachineReg, int> mregPair(mreg, size);

                    // add pair to 2 maps
                    m_mapMregVreg.insert(std::pair<MachineReg, int>(mreg, vreg));
                    m_mapVregMreg.insert(std::pair<int, std::pair<MachineReg, int>>(vreg, mregPair));

                    // set to new operand
                    Operand newOperand(operand.get_kind(), operand.get_base_reg());
                    newOperand.setMreg(mregPair);
                    newIns->setOperand(j, newOperand);
//                    std::printf("vreg %d mapped to mreg %d, mregPair<%d, %d>\n", vreg, (int)mreg, (int)mreg, size);
                }
            }
        }
        new_bb->append(newIns);
//        printMapVec();
    }

//    std::printf("\n\nstart printing block %d============\n\n", orig_bb->get_id());
//    PrintHighLevelCode().print_instructions(new_bb);
//    std::printf("\n\nend printing block %d============\n\n", orig_bb->get_id());
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

//    std::printf("\n%s\n", m_liveVregAll.fact_to_string(factInsBef).c_str());
//    std::printf("%s\n", HighLevelFormatter().format_instruction(ins).c_str());
//    std::printf("%s\n", m_liveVregAll.fact_to_string(factInsAft).c_str());
//    printMapVec();

    for (auto iter = m_mapMregVreg.begin(); iter != m_mapMregVreg.end();) {
        if (!factInsBef.test(iter->second)) {
            // dead vreg
            // erase from 2 maps
//            std::printf("vreg %d died, mreg %d released\n", iter->second, (int)iter->first);
            m_availMregs.push_back(iter->first);
            auto iter_mapVregMreg = m_mapVregMreg.find(iter->second);
            assert(iter_mapVregMreg != m_mapVregMreg.end());
            m_mapVregMreg.erase(iter_mapVregMreg);
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

//std::map<MachineReg, int>::iterator LocalRegAllocation::getIterAllocatedVreg(int vreg) {
//    std::map<MachineReg, int>::iterator iter;
//    for (iter = m_mapMregVreg.begin(); iter != m_mapMregVreg.end(); ++iter) {
//        if (iter->second == vreg) return iter;
//    }
//    return iter;
//}

void LocalRegAllocation::printMapVec() {
    // print 2 maps and vector
    std::printf("\nmapMregVreg===========\n");
    for (auto iter = m_mapMregVreg.begin(); iter != m_mapMregVreg.end(); ++iter) {
        std::printf("(%d, %d), ", (int)iter->first, iter->second);
    }
    std::printf("\nmapVregMreg===========\n");
    for (auto iter = m_mapVregMreg.begin(); iter != m_mapVregMreg.end(); ++iter) {
        std::printf("(%d, ", (int)iter->first);
        std::printf("<%d, %d>), ", (int)iter->second.first, iter->second.second);
    }
    std::printf("\nvecAvailMreg===========\n");
    for (auto iter = m_availMregs.begin(); iter != m_availMregs.end(); ++iter) {
        std::printf("%d, ", (int)*iter);
    }
    std::printf("\n\n");
}

//Operand LocalRegAllocation::setMregOperand() {
//
//}

void LocalRegAllocation::calSpillAddr(const BasicBlock *orig_bb) {
    int maxAvail = 0;
    for (auto i = orig_bb->cbegin(); i != orig_bb->cend(); ++i) {
//        LiveVregs::FactType factInsBef = m_liveVregAll.get_fact_before_instruction(orig_bb, *i);
        int tempAvail = 0;
        LiveVregs::FactType factInsAft = m_liveVregAll.get_fact_after_instruction(orig_bb, *i);
        for (unsigned j = 0; j < LiveVregsAnalysis::MAX_VREGS; j++) {
            if (factInsAft.test(j)
                    && !m_factBeg.test(j)
                    && !m_factEnd.test(j)
                    && !m_funcVars.test(j)) {
                // a vreg that needs to be allocated in mreg
                tempAvail += 1;
            }
        }
        maxAvail = std::max(tempAvail, maxAvail);
    }
    // no need to spill
    if (maxAvail == 0) return;
    // need to spill
    for (auto i = 0; i < maxAvail; i++) {
        m_spillAddr.push_back()
    }
}
