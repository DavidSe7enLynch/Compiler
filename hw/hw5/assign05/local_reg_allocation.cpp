//
// Created by Ruirong Huang on 12/4/22.
//

#include "local_reg_allocation.h"

LocalRegAllocation::LocalRegAllocation(const std::shared_ptr <ControlFlowGraph> &cfg, const std::shared_ptr <InstructionSequence> &hl_iseq)
        : ControlFlowGraphTransform(cfg), m_liveVregAll(cfg), m_localStorageClass1(hl_iseq->get_funcdef_ast()->getLocalTotalStorage()) {
    m_liveVregAll.execute();
    for (int i = 0; i < hl_iseq->getCalleeVec().size(); i++) {
        int vreg = hl_iseq->getCalleeVec().at(i).first;
        m_funcVars.set(vreg);
        // only first 4 can be allocated with mreg r12-r15
        if (i < 4) {
            m_mregFuncVars.set(vreg);
        }
    }
}

std::shared_ptr <InstructionSequence> LocalRegAllocation::transform_basic_block(const BasicBlock *orig_bb) {
    // init
    // only deal with normal blocks with instructions
    // get live regs at beg/end
    // get available mregs
    // clear maps
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
    m_mapSpilledMregVreg.clear();
    m_mapSpilledVregMreg.clear();

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

            // if a class 2 vreg, map it to memory
            if (m_memVreg.test(vreg)) {
                assert(m_mapVregMemClass2.count(vreg));
                newIns->get_operand(j).setMemAddr(m_mapVregMemClass2.at(vreg));
            }

            if (!m_factBeg.test(vreg)
                    && !m_factEnd.test(vreg)
                    && !m_funcVars.test(vreg)
                    && vreg > 9) {
                // get a temp vreg, store it in one avail mreg
                std::map<int, std::pair<MachineReg, int>>::iterator iter = m_mapVregMreg.find(vreg);
                std::map<int, std::pair<MachineReg, int>>::iterator iterSpill = m_mapSpilledVregMreg.find(vreg);
                if (iterSpill != m_mapSpilledVregMreg.end()) {
                    // corresponds to a spilled mreg
                    // TODO
                } else if (iter != m_mapVregMreg.end()) {
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
                    std::printf("\n\n\n\n============Warning, no available mreg=============\n\n\n\n");
                    // make sure there is reserved places for spilled mreg
                    assert(m_spillAddr.size() > 0);

                    // steps
                    // choose a used mreg
                    // insert it with its corresponding vreg to m_mapSpillMregVreg
                    // insert a <vreg, <mreg, size>> pair to m_mapSpillVregMreg
                    // remove these 2 from 2 maps: m_mapMregVreg, m_mapVregMreg
                    // do not modify m_availMregs, since we immediately consume that mreg

                    // add an instruction to spill the mreg
                    // modify current instruction as normal allocation process
                    // TODO
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
                    if(m_isPrint) std::printf("vreg %d mapped to mreg %d, mregPair<%d, %d>\n", vreg, (int)mreg, (int)mreg, size);
                }
            }
        }
        new_bb->append(newIns);

        if(m_isPrint) printMapVec();
    }

    if(m_isPrint) {
        std::printf("\n\nstart printing block %d============\n\n", orig_bb->get_id());
        PrintHighLevelCode().print_instructions(new_bb);
        std::printf("\n\nend printing block %d============\n\n", orig_bb->get_id());
    }
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

    if(m_isPrint) {
        std::printf("\n%s\n", m_liveVregAll.fact_to_string(factInsBef).c_str());
        std::printf("%s\n", HighLevelFormatter().format_instruction(ins).c_str());
        std::printf("%s\n", m_liveVregAll.fact_to_string(factInsAft).c_str());
        printMapVec();
    }

    for (auto iter = m_mapMregVreg.begin(); iter != m_mapMregVreg.end();) {
        if (!factInsBef.test(iter->second)) {
            // dead vreg
            // erase from 2 maps
            if(m_isPrint) std::printf("vreg %d died, mreg %d released\n", iter->second, (int)iter->first);
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

std::shared_ptr <ControlFlowGraph> LocalRegAllocation::transform_cfg() {
    std::shared_ptr <ControlFlowGraph> result(new ControlFlowGraph());

    // map of basic blocks of original CFG to basic blocks in transformed CFG
    std::map < BasicBlock * , BasicBlock * > block_map;

    // allocate memory
    allocateMemory();

    // iterate over all basic blocks, transforming each one
    for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++) {
        BasicBlock *orig = *i;

        // Transform the instructions
        std::shared_ptr <InstructionSequence> transformed_bb = transform_basic_block(orig);

        // Create transformed basic block; note that we set its
        // code order to be the same as the code order of the original
        // block (with the hope of eventually reconstructing an InstructionSequence
        // with the transformed blocks in an order that matches the original
        // block order)
        BasicBlock *result_bb = result->create_basic_block(orig->get_kind(), orig->get_code_order(), orig->get_label());
        for (auto j = transformed_bb->cbegin(); j != transformed_bb->cend(); ++j)
            result_bb->append((*j)->duplicate());

        block_map[orig] = result_bb;
    }

    // add edges to transformed CFG
    for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++) {
        BasicBlock *orig = *i;
        const ControlFlowGraph::EdgeList &outgoing_edges = m_cfg->get_outgoing_edges(orig);
        for (auto j = outgoing_edges.cbegin(); j != outgoing_edges.cend(); j++) {
            Edge *orig_edge = *j;

            BasicBlock *transformed_source = block_map[orig_edge->get_source()];
            BasicBlock *transformed_target = block_map[orig_edge->get_target()];

            result->create_edge(transformed_source, transformed_target, orig_edge->get_kind());
        }
    }

    result->setTotalMemory(m_totalMemory);
    if(m_isPrint) std::printf("set total memory: %d\n", m_totalMemory);
    return result;
}

void LocalRegAllocation::calMemVreg() {
    for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++) {
        m_factBeg = m_liveVregAll.get_fact_at_beginning_of_block(*i);
        m_factEnd = m_liveVregAll.get_fact_at_end_of_block(*i);

        // class 2: (alive at beg/end of blocks && !func vars) || !mreg_allocated func vars
        // do not consider vr0-vr9
        for (auto j = 10; j < LiveVregsAnalysis::MAX_VREGS; j++) {
            if (((m_factBeg.test(j) || m_factEnd.test(j)) && !m_funcVars.test(j))
                    || (m_funcVars.test(j) && !m_mregFuncVars.test(j))) {
                m_memVreg.set(j);
                if(m_isPrint) std::printf("vreg %d is class 2, shall store in mem\n", j);
            }
        }
    }
}

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
void LocalRegAllocation::allocateMemory() {
    // allocate memory for class 2 vregs
    // steps:
    // get class 2 vregs
    // get starting point: memClass1
    // set mem (8 byte) for each class 2 vreg

    calMemVreg();

    long memClass1 = m_localStorageClass1;
    if ((memClass1) % 16 != 0) {
        memClass1 += (16 - (memClass1 % 16));
    }

    long memOffset = memClass1;
    for (auto i = 0; i < LiveVregsAnalysis::MAX_VREGS; i++) {
        if (m_memVreg.test(i)) {
            // is class 2 vreg
            memOffset += 8;
            m_mapVregMemClass2.insert(std::pair<int, long>(i, -memOffset));
        }
    }
    if(m_isPrint) std::printf("class1mem: %ld, class1+2mem: %ld, ", memClass1, memOffset);
    // reserve memory for class 3 spilled vregs
    // calculate max place needed for class 3: spilled vregs
    int maxSpill = calMaxSpill();
    for (auto i = 0; i < maxSpill; i++) {
        memOffset += 8;
        m_spillAddr.push_back(memOffset);
    }
    m_totalMemory = memOffset;
    if(m_isPrint) std::printf("maxSpill: %d, total memory: %ld\n", maxSpill, m_totalMemory);
}

int LocalRegAllocation::calMaxSpill() {
    // calculate max place needed for class 3: spilled vregs
    int maxSpill = 0;
    for (auto iter = m_cfg->bb_begin(); iter != m_cfg->bb_end(); iter++) {
        BasicBlock *bb = *iter;
        if (bb->get_kind() != BASICBLOCK_INTERIOR || bb->get_length() == 0) {
            continue;
        }
        initAvailMregs(bb);
        m_factBeg = m_liveVregAll.get_fact_at_beginning_of_block(bb);
        m_factEnd = m_liveVregAll.get_fact_at_end_of_block(bb);
        int bbAlive = 0;
        for (auto i = bb->cbegin(); i != bb->cend(); ++i) {
//            LiveVregs::FactType factInsBef = m_liveVregAll.get_fact_before_instruction(bb, *i);
            LiveVregs::FactType factInsAft = m_liveVregAll.get_fact_after_instruction(bb, *i);
            int insAlive = 0;
            for (unsigned j = 0; j < LiveVregsAnalysis::MAX_VREGS; j++) {
                if (factInsAft.test(j)
                    && !m_factBeg.test(j)
                    && !m_factEnd.test(j)
                    && !m_funcVars.test(j)) {
                    // a vreg that needs to be allocated in mreg
                    insAlive += 1;
                }
            }
            bbAlive = std::max(bbAlive, insAlive);
        }
        int bbSpill = bbAlive - m_availMregs.size();
        maxSpill = std::max(maxSpill, bbSpill);
    }
    return maxSpill;
}

