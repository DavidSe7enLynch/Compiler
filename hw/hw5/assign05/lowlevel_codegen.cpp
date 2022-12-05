#include <cassert>
#include "node.h"
#include "instruction.h"
#include "operand.h"
#include "local_storage_allocation.h"
#include "highlevel.h"
#include "exceptions.h"
#include "lowlevel_codegen.h"
#include "cfg.h"
#include "highlevel_local_optimize.h"
#include "local_reg_allocation.h"

namespace {

// This map has some "obvious" translations of high-level opcodes to
// low-level opcodes.
    const std::map <HighLevelOpcode, LowLevelOpcode> HL_TO_LL = {
            {HINS_nop,      MINS_NOP},
            {HINS_add_b,    MINS_ADDB},
            {HINS_add_w,    MINS_ADDW},
            {HINS_add_l,    MINS_ADDL},
            {HINS_add_q,    MINS_ADDQ},
            {HINS_sub_b,    MINS_SUBB},
            {HINS_sub_w,    MINS_SUBW},
            {HINS_sub_l,    MINS_SUBL},
            {HINS_sub_q,    MINS_SUBQ},
            {HINS_mul_l,    MINS_IMULL},
            {HINS_mul_q,    MINS_IMULQ},
            {HINS_mov_b,    MINS_MOVB},
            {HINS_mov_w,    MINS_MOVW},
            {HINS_mov_l,    MINS_MOVL},
            {HINS_mov_q,    MINS_MOVQ},
            {HINS_sconv_bw, MINS_MOVSBW},
            {HINS_sconv_bl, MINS_MOVSBL},
            {HINS_sconv_bq, MINS_MOVSBQ},
            {HINS_sconv_wl, MINS_MOVSWL},
            {HINS_sconv_wq, MINS_MOVSWQ},
            {HINS_sconv_lq, MINS_MOVSLQ},
            {HINS_uconv_bw, MINS_MOVZBW},
            {HINS_uconv_bl, MINS_MOVZBL},
            {HINS_uconv_bq, MINS_MOVZBQ},
            {HINS_uconv_wl, MINS_MOVZWL},
            {HINS_uconv_wq, MINS_MOVZWQ},
            {HINS_uconv_lq, MINS_MOVZLQ},
            {HINS_ret,      MINS_RET},
            {HINS_jmp,      MINS_JMP},
            {HINS_call,     MINS_CALL},

            // For comparisons, it is expected that the code generator will first
            // generate a cmpb/cmpw/cmpl/cmpq instruction to compare the operands,
            // and then generate a setXX instruction to put the result of the
            // comparison into the destination operand. These entries indicate
            // the apprpropriate setXX instruction to use.
            {HINS_cmplt_b,  MINS_SETL},
            {HINS_cmplt_w,  MINS_SETL},
            {HINS_cmplt_l,  MINS_SETL},
            {HINS_cmplt_q,  MINS_SETL},
            {HINS_cmplte_b, MINS_SETLE},
            {HINS_cmplte_w, MINS_SETLE},
            {HINS_cmplte_l, MINS_SETLE},
            {HINS_cmplte_q, MINS_SETLE},
            {HINS_cmpgt_b,  MINS_SETG},
            {HINS_cmpgt_w,  MINS_SETG},
            {HINS_cmpgt_l,  MINS_SETG},
            {HINS_cmpgt_q,  MINS_SETG},
            {HINS_cmpgte_b, MINS_SETGE},
            {HINS_cmpgte_w, MINS_SETGE},
            {HINS_cmpgte_l, MINS_SETGE},
            {HINS_cmpgte_q, MINS_SETGE},
            {HINS_cmpeq_b,  MINS_SETE},
            {HINS_cmpeq_w,  MINS_SETE},
            {HINS_cmpeq_l,  MINS_SETE},
            {HINS_cmpeq_q,  MINS_SETE},
            {HINS_cmpneq_b, MINS_SETNE},
            {HINS_cmpneq_w, MINS_SETNE},
            {HINS_cmpneq_l, MINS_SETNE},
            {HINS_cmpneq_q, MINS_SETNE},
    };

}

LowLevelCodeGen::LowLevelCodeGen(bool optimize)
        : m_total_memory_storage(0), m_optimize(optimize) {
}

LowLevelCodeGen::~LowLevelCodeGen() {
}

std::shared_ptr <InstructionSequence> LowLevelCodeGen::generate(const std::shared_ptr <InstructionSequence> &hl_iseq) {
    // TODO: if optimizations are enabled, could do analysis/transformation of high-level code

    Node *funcdef_ast = hl_iseq->get_funcdef_ast();

    // cur_hl_iseq is the "current" version of the high-level IR,
    // which could be a transformed version if we are doing optimizations
    std::shared_ptr<InstructionSequence> cur_hl_iseq(hl_iseq);

    if (m_optimize) {
        // High-level optimizations

        // Create a control-flow graph representation of the high-level code
        HighLevelControlFlowGraphBuilder hl_cfg_builder(cur_hl_iseq);
        std::shared_ptr<ControlFlowGraph> cfg = hl_cfg_builder.build();

        // Do local optimizations
        // LVN
        HighlevelLocalOptimize hl_opts(cfg);
        cfg = hl_opts.transform_cfg();

        // local reg allocation
        LocalRegAllocation lra(cfg, hl_iseq);
        // TODO: replace lra_cfg with cfg
        std::shared_ptr<ControlFlowGraph> lra_cfg = lra.transform_cfg();

        // Convert the transformed high-level CFG back to an InstructionSequence
        cur_hl_iseq = cfg->create_instruction_sequence();

        // The function definition AST might have information needed for
        // low-level code generation
        cur_hl_iseq->set_funcdef_ast(funcdef_ast);
    }

//    std::printf("\n\nstart printing orig_highlevel code============\n\n");
//    PrintHighLevelCode().print_instructions(hl_iseq);
//    std::printf("\n\nend printing orig_highlevel code============\n\n");
//
//    std::printf("\n\nstart printing new_highlevel code============\n\n");
//    PrintHighLevelCode().print_instructions(cur_hl_iseq);
//    std::printf("\n\nend printing new_highlevel code============\n\n");

    // keep info for global reg alloc
    cur_hl_iseq->setCalleeVec(hl_iseq->getCalleeVec());
    std::shared_ptr <InstructionSequence> ll_iseq = translate_hl_to_ll(cur_hl_iseq);

    // TODO: if optimizations are enabled, could do analysis/transformation of low-level code

    return ll_iseq;
}

std::shared_ptr <InstructionSequence>
LowLevelCodeGen::translate_hl_to_ll(const std::shared_ptr <InstructionSequence> &hl_iseq) {
    std::shared_ptr <InstructionSequence> ll_iseq(new InstructionSequence());

    // The high-level InstructionSequence will have a pointer to the Node
    // representing the function definition. Useful information could be stored
    // there (for example, about the amount of memory needed for local storage,
    // maximum number of virtual registers used, etc.)
    Node *funcdef_ast = hl_iseq->get_funcdef_ast();
    assert(funcdef_ast != nullptr);

    // It's not a bad idea to store the pointer to the function definition AST
    // in the low-level InstructionSequence as well, in case it's needed by
    // optimization passes.
    ll_iseq->set_funcdef_ast(funcdef_ast);

    // Determine the total number of bytes of memory storage
    // that the function needs. This should include both variables that
    // *must* have storage allocated in memory (e.g., arrays), and also
    // any additional memory that is needed for virtual registers,
    // spilled machine registers, etc.
    m_localStorage = ll_iseq->get_funcdef_ast()->getLocalTotalStorage();
    int numVreg = 0;
    for (auto slot = hl_iseq->cbegin(); slot != hl_iseq->cend(); slot++) {
        Instruction *ins = *slot;
        int numOperand = ins->get_num_operands();
        for (auto i = 0; i < numOperand; i++) {
            Operand operand = ins->get_operand(i);
            if (!operand.is_non_reg()) {
                int vreg = operand.get_base_reg();
                if (vreg > numVreg)
                    numVreg = vreg;
            }
        }
    }
    m_total_memory_storage = m_localStorage + (numVreg - 9) * 8;
//    std::printf("m_total_memory_storage: %d\n", m_total_memory_storage);

    // The function prologue will push %rbp, which should guarantee that the
    // stack pointer (%rsp) will contain an address that is a multiple of 16.
    // If the total memory storage required is not a multiple of 16, add to
    // it so that it is.
    if ((m_total_memory_storage) % 16 != 0)
        m_total_memory_storage += (16 - (m_total_memory_storage % 16));


    if (m_optimize) {
        // push callee-saved regs to stack
        // depend on size of occurVars, max 5
        // order: r12-r15, rbx
        // pop back before ret
        // also set the vreg-mreg map for global allocation
        m_numPush = std::min(static_cast<int>(hl_iseq->m_occurVarsVec.size()), 5);
        for (int i = 0; i < m_numPush; i++) {
            Operand operand = Operand(Operand::MREG64, m_callee[i]);
            ll_iseq->append(new Instruction(MINS_PUSHQ, operand));
            m_mregMap.insert(std::pair<int, MachineReg>(hl_iseq->m_occurVarsVec[i].first, m_callee[i]));
        }
    }

    // Iterate through high level instructions
    for (auto i = hl_iseq->cbegin(); i != hl_iseq->cend(); ++i) {
        Instruction *hl_ins = *i;

        // If the high-level instruction has a label, define an equivalent
        // label in the low-level instruction sequence
        if (i.has_label())
            ll_iseq->define_label(i.get_label());

        // Translate the high-level instruction into one or more low-level instructions
        translate_instruction(hl_ins, ll_iseq);
    }

    return ll_iseq;
}

namespace {

// These helper functions are provided to make it easier to handle
// the way that instructions and operands vary based on operand size
// ('b'=1 byte, 'w'=2 bytes, 'l'=4 bytes, 'q'=8 bytes.)

// Check whether hl_opcode matches a range of opcodes, where base
// is a _b variant opcode. Return true if the hl opcode is any variant
// of that base.
    bool match_hl(int base, int hl_opcode) {
        return hl_opcode >= base && hl_opcode < (base + 4);
    }

    bool matchHlConv(int hlOpcode) {
        int base = HINS_sconv_bw;
//        std::printf("in matchHlConv, base = %d, hlOpcode = %d\n", base, hlOpcode);
        return hlOpcode >= base && hlOpcode < (base + 12);
    }

// For a low-level instruction with 4 size variants, return the correct
// variant. base_opcode should be the "b" variant, and operand_size
// should be the operand size in bytes (1, 2, 4, or 8.)
    LowLevelOpcode select_ll_opcode(LowLevelOpcode base_opcode, int operand_size) {
        int off;

        switch (operand_size) {
            case 1: // 'b' variant
                off = 0;
                break;
            case 2: // 'w' variant
                off = 1;
                break;
            case 4: // 'l' variant
                off = 2;
                break;
            case 8: // 'q' variant
                off = 3;
                break;
            default:
                assert(false);
                off = 3;
        }

        return LowLevelOpcode(int(base_opcode) + off);
    }

// Get the correct Operand::Kind value for a machine register
// of the specified size (1, 2, 4, or 8 bytes.)
    Operand::Kind select_mreg_kind(int operand_size) {
        switch (operand_size) {
            case 1:
                return Operand::MREG8;
            case 2:
                return Operand::MREG16;
            case 4:
                return Operand::MREG32;
            case 8:
                return Operand::MREG64;
            default:
                assert(false);
                return Operand::MREG64;
        }
    }

}

void LowLevelCodeGen::translate_instruction(Instruction *hl_ins, const std::shared_ptr <InstructionSequence> &ll_iseq) {
    HighLevelOpcode hl_opcode = HighLevelOpcode(hl_ins->get_opcode());

    m_ifUseRAX = false;

    if (hl_opcode == HINS_enter) {
//        std::printf("in enter\n");
        // Function prologue: this will create an ABI-compliant stack frame.
        // The local variable area is *below* the address in %rbp, and local storage
        // can be accessed at negative offsets from %rbp. For example, the topmost
        // 4 bytes in the local storage area are at -4(%rbp).
        ll_iseq->append(new Instruction(MINS_PUSHQ, Operand(Operand::MREG64, MREG_RBP)));
        ll_iseq->append(
                new Instruction(MINS_MOVQ, Operand(Operand::MREG64, MREG_RSP), Operand(Operand::MREG64, MREG_RBP)));
        ll_iseq->append(new Instruction(MINS_SUBQ, Operand(Operand::IMM_IVAL, m_total_memory_storage),
                                        Operand(Operand::MREG64, MREG_RSP)));

        return;
    }

    if (hl_opcode == HINS_leave) {
        // Function epilogue: deallocate local storage area and restore original value
        // of %rbp

        // set label
//        ll_iseq->define_label()

        ll_iseq->append(new Instruction(MINS_ADDQ, Operand(Operand::IMM_IVAL, m_total_memory_storage),
                                        Operand(Operand::MREG64, MREG_RSP)));
        ll_iseq->append(new Instruction(MINS_POPQ, Operand(Operand::MREG64, MREG_RBP)));

        return;
    }

    if (hl_opcode == HINS_ret) {
        if (m_optimize) {
            // pop callee-saved regs from stack
            // reverse order
            for (int i = 0; i < m_numPush; i++) {
                Operand operand = Operand(Operand::MREG64, m_callee[m_numPush - i - 1]);
                ll_iseq->append(new Instruction(MINS_POPQ, operand));
            }
        }
        ll_iseq->append(new Instruction(MINS_RET));
        return;
    }

    // TODO: handle other high-level instructions
    // Note that you can use the highlevel_opcode_get_source_operand_size() and
    // highlevel_opcode_get_dest_operand_size() functions to determine the
    // size (in bytes, 1, 2, 4, or 8) of either the source operands or
    // destination operand of a high-level instruction. This should be useful
    // for choosing the appropriate low-level instructions and
    // machine register operands.

    if (match_hl(HINS_mov_b, hl_opcode)) {
        int size = highlevel_opcode_get_source_operand_size(hl_opcode);

        LowLevelOpcode mov_opcode = select_ll_opcode(MINS_MOVB, size);

        Operand src_operand = get_ll_operand(hl_ins->get_operand(1), size, ll_iseq);
        Operand dest_operand = get_ll_operand(hl_ins->get_operand(0), size, ll_iseq);

        if (src_operand.is_memref() && dest_operand.is_memref()) {
            // move source operand into a temporary register
            Operand::Kind mreg_kind = select_mreg_kind(size);
            Operand r10(mreg_kind, MREG_R10);
            ll_iseq->append(new Instruction(mov_opcode, src_operand, r10));
            src_operand = r10;
        }

        ll_iseq->append(new Instruction(mov_opcode, src_operand, dest_operand));
        return;
    }

    if (match_hl(HINS_add_b, hl_opcode)
        || match_hl(HINS_sub_b, hl_opcode)) {
        int destSize = highlevel_opcode_get_dest_operand_size(hl_opcode);

//        std::printf("in add\n");
        // step:
        // 1. mov srcl to dest
        // 2: add srcr to dest

        LowLevelOpcode movOpcode = select_ll_opcode(MINS_MOVB, destSize);

        LowLevelOpcode addOpcode;
        if (match_hl(HINS_add_b, hl_opcode)) addOpcode = select_ll_opcode(MINS_ADDB, destSize);
        else if (match_hl(HINS_sub_b, hl_opcode)) addOpcode = select_ll_opcode(MINS_SUBB, destSize);

        Operand srcOperandl = get_ll_operand(hl_ins->get_operand(1), destSize, ll_iseq);
        Operand srcOperandr = get_ll_operand(hl_ins->get_operand(2), destSize, ll_iseq);
        Operand destOperand = get_ll_operand(hl_ins->get_operand(0), destSize, ll_iseq);

        if ((srcOperandl.is_memref() || srcOperandr.is_memref()) && destOperand.is_memref()) {
            Operand::Kind mreg_kind = select_mreg_kind(destSize);
            Operand r10(mreg_kind, MREG_R10);
            ll_iseq->append(new Instruction(movOpcode, srcOperandl, r10));
            ll_iseq->append(new Instruction(addOpcode, srcOperandr, r10));
            ll_iseq->append(new Instruction(movOpcode, r10, destOperand));
        } else {
            ll_iseq->append(new Instruction(movOpcode, srcOperandl, destOperand));
            ll_iseq->append(new Instruction(addOpcode, srcOperandr, destOperand));
        }
        return;
    }

    if (match_hl(HINS_mul_b, hl_opcode)
        /* || match_hl(HINS_div_b, hl_opcode) */) {
//        std::printf("in mul\n");
        // steps:
        // mov
        // imul/idiv
        // mov

        int size = highlevel_opcode_get_source_operand_size(hl_opcode);
        assert(size == 4 || size == 8);

        LowLevelOpcode movOpcode = select_ll_opcode(MINS_MOVB, size);
        LowLevelOpcode imulOpcode;
        if (match_hl(HINS_mul_b, hl_opcode)) imulOpcode = size == 4 ? MINS_IMULL : MINS_IMULQ;
//        else if (match_hl(HINS_div_b, hl_opcode)) imulOpcode = size == 4 ? MINS_IDIVL: MINS_IDIVL;

        Operand srcOperandl = get_ll_operand(hl_ins->get_operand(1), size, ll_iseq);
        Operand srcOperandr = get_ll_operand(hl_ins->get_operand(2), size, ll_iseq);
        Operand destOperand = get_ll_operand(hl_ins->get_operand(0), size, ll_iseq);

        Operand::Kind mreg_kind = select_mreg_kind(size);
        Operand r10(mreg_kind, MREG_R10);
        ll_iseq->append(new Instruction(movOpcode, srcOperandl, r10));

//        if (srcOperandr.is_memref()) {
//            Operand::Kind mreg_kind = select_mreg_kind(size);
//            Operand r11(mreg_kind, MREG_R11);
//            ll_iseq->append(new Instruction(movOpcode, srcOperandr, r11));
//            srcOperandr = r11;
//        }
        ll_iseq->append(new Instruction(imulOpcode, srcOperandr, r10));
        ll_iseq->append(new Instruction(movOpcode, r10, destOperand));
        return;
    }

    if (match_hl(HINS_div_b, hl_opcode) || match_hl(HINS_mod_b, hl_opcode)) {
        // steps:
        // mov srcl %eax
        // cdq
        // mov srcr %r10
        // idiv %r10
        // mov %eax/%edx dest

        int size = highlevel_opcode_get_source_operand_size(hl_opcode);

        LowLevelOpcode movOpcode = select_ll_opcode(MINS_MOVB, size);
        LowLevelOpcode cdqOpcode = MINS_CDQ;
        LowLevelOpcode idivOpcode = size == 4 ? MINS_IDIVL : MINS_IDIVQ;

        Operand srcOperandl = get_ll_operand(hl_ins->get_operand(1), size, ll_iseq);
        Operand srcOperandr = get_ll_operand(hl_ins->get_operand(2), size, ll_iseq);
        Operand destOperand = get_ll_operand(hl_ins->get_operand(0), size, ll_iseq);

        Operand::Kind mreg_kind = select_mreg_kind(size);
        Operand r10(mreg_kind, MREG_R10);

        Operand srcLTempOperand = Operand(select_mreg_kind(size), 0);

        Operand resultOperand;
        if (match_hl(HINS_div_b, hl_opcode)) {
            resultOperand = Operand(select_mreg_kind(size), 0);
        } else {
            resultOperand = Operand(select_mreg_kind(size), 3);
        }

        ll_iseq->append(new Instruction(movOpcode, srcOperandl, srcLTempOperand));
        ll_iseq->append(new Instruction(cdqOpcode));
        ll_iseq->append(new Instruction(movOpcode, srcOperandr, r10));
        ll_iseq->append(new Instruction(idivOpcode, r10));
        ll_iseq->append(new Instruction(movOpcode, resultOperand, destOperand));
        return;
    }

    if (hl_opcode == HINS_jmp) {
        LowLevelOpcode jmpOpcode = MINS_JMP;
        Operand labelOperand = get_ll_operand(hl_ins->get_operand(0), 0, ll_iseq);
        ll_iseq->append(new Instruction(jmpOpcode, labelOperand));
        return;
    }

    if (match_hl(HINS_cmplte_b, hl_opcode)
        || match_hl(HINS_cmplt_b, hl_opcode)
        || match_hl(HINS_cmpgte_b, hl_opcode)
        || match_hl(HINS_cmpgt_b, hl_opcode)
        || match_hl(HINS_cmpeq_b, hl_opcode)
        || match_hl(HINS_cmpneq_b, hl_opcode)) {
        // step:
        // 1. mov left vreg(memory) to r10
        // 2. cmp right vreg(mem) to r10
        // 3: set to r10 (le here)
        // 4. movzbl r10 to r11 (just an example)
        // 5. mov r11 to vreg(memory)

        int cmpSize = highlevel_opcode_get_source_operand_size(hl_opcode);
        LowLevelOpcode movOpcode = select_ll_opcode(MINS_MOVB, cmpSize);
        LowLevelOpcode cmpOpcode = select_ll_opcode(MINS_CMPB, cmpSize);

        LowLevelOpcode setOpcode;
        if (match_hl(HINS_cmplte_b, hl_opcode)) setOpcode = MINS_SETLE;
        else if (match_hl(HINS_cmplt_b, hl_opcode)) setOpcode = MINS_SETL;
        else if (match_hl(HINS_cmpgte_b, hl_opcode)) setOpcode = MINS_SETGE;
        else if (match_hl(HINS_cmpgt_b, hl_opcode)) setOpcode = MINS_SETG;
        else if (match_hl(HINS_cmpeq_b, hl_opcode)) setOpcode = MINS_SETE;
        else if (match_hl(HINS_cmpneq_b, hl_opcode)) setOpcode = MINS_SETNE;

        int offset = cmpSize / 2;
        assert(offset == 1 || offset == 2 || offset == 4);
        LowLevelOpcode movzblOpcode = select_ll_opcode(MINS_MOVZBW, cmpSize / 2);

        Operand srcOperandl = get_ll_operand(hl_ins->get_operand(1), cmpSize, ll_iseq);
        Operand srcOperandr = get_ll_operand(hl_ins->get_operand(2), cmpSize, ll_iseq);
        Operand destOperand = get_ll_operand(hl_ins->get_operand(0), cmpSize, ll_iseq);

        Operand::Kind mreg_kind = select_mreg_kind(cmpSize);
        Operand r10(mreg_kind, MREG_R10);
        Operand r11(mreg_kind, MREG_R11);
        Operand r10b(Operand::MREG8, MREG_R10);

        ll_iseq->append(new Instruction(movOpcode, srcOperandl, r10));
        ll_iseq->append(new Instruction(cmpOpcode, srcOperandr, r10));
        ll_iseq->append(new Instruction(setOpcode, r10b));
        ll_iseq->append(new Instruction(movzblOpcode, r10b, r11));
        ll_iseq->append(new Instruction(movOpcode, r11, destOperand));
        return;
    }

    if (hl_opcode == HINS_cjmp_t || hl_opcode == HINS_cjmp_f) {
        // steps:
        // cmp $0, dest
        // jne label

        LowLevelOpcode cmpOpcode = select_ll_opcode(MINS_CMPB, 4); // l for imm

        LowLevelOpcode jneOpcode;
        if (hl_opcode == HINS_cjmp_t) jneOpcode = MINS_JNE;
        else if (hl_opcode == HINS_cjmp_f) jneOpcode = MINS_JE;

        // $0
        Operand operand0(Operand::IMM_IVAL, 0);
        // dest
        Operand operandl = get_ll_operand(hl_ins->get_operand(0), 4, ll_iseq);
        // label
        Operand operandr = get_ll_operand(hl_ins->get_operand(1), 0, ll_iseq);

        ll_iseq->append(new Instruction(cmpOpcode, operand0, operandl));
        ll_iseq->append(new Instruction(jneOpcode, operandr));

        return;
    }

    if (hl_opcode == HINS_call) {
        LowLevelOpcode callOpcode = MINS_CALL;
        Operand operandCall = get_ll_operand((hl_ins->get_operand(0)), 0, ll_iseq);
        ll_iseq->append(new Instruction(callOpcode, operandCall));
        return;
    }

    if (hl_opcode == HINS_localaddr) {
//        std::printf("in localaddr\n");
        // steps:
        // get the bottom of local storage
        // get index of this address

        // leaq
        // movq

        long idx = hl_ins->get_operand(1).get_imm_ival() - m_localStorage;
        // %rbp: 7
        Operand addrOperand = Operand(Operand::MREG64_MEM_OFF, 7, idx);
        Operand r10(Operand::MREG64, MREG_R10);
        Operand destOperand = get_ll_operand(hl_ins->get_operand(0), 8, ll_iseq);

        ll_iseq->append(new Instruction(MINS_LEAQ, addrOperand, r10));
        ll_iseq->append(new Instruction(MINS_MOVQ, r10, destOperand));
        return;
    }

    if (matchHlConv(hl_opcode)) {
//        std::printf("in conv\n");
        // steps:
        // mov
        // movslq
        // mov

        int idx = int(hl_opcode) - int(HINS_sconv_bw);
        int srcSize = highlevel_opcode_get_source_operand_size(hl_opcode);
        int destSize = highlevel_opcode_get_dest_operand_size(hl_opcode);
        assert(srcSize < destSize);

        LowLevelOpcode movSrcOpcode = select_ll_opcode(MINS_MOVB, srcSize);
        LowLevelOpcode movDestOpcode = select_ll_opcode(MINS_MOVB, destSize);
        LowLevelOpcode movslqOpcode = LowLevelOpcode(int(MINS_MOVSBW) + idx);

        Operand srcOperand = get_ll_operand(hl_ins->get_operand(1), srcSize, ll_iseq);
        Operand destOperand = get_ll_operand(hl_ins->get_operand(0), destSize, ll_iseq);

        Operand::Kind srcMregKind = select_mreg_kind(srcSize);
        Operand r10src(srcMregKind, MREG_R10);
        Operand::Kind destMregKind = select_mreg_kind(destSize);
        Operand r10dest(destMregKind, MREG_R10);

        ll_iseq->append(new Instruction(movSrcOpcode, srcOperand, r10src));
        ll_iseq->append(new Instruction(movslqOpcode, r10src, r10dest));
        ll_iseq->append(new Instruction(movDestOpcode, r10dest, destOperand));
        return;
    }

    if (match_hl(HINS_neg_b, hl_opcode)) {
        // neg vrdest vrsrc
        // steps
        // mov vrsrc r10
        // mov $0 vrdest
        // sub r10 vrdest

        int size = highlevel_opcode_get_source_operand_size(hl_opcode);

        LowLevelOpcode movOpcode = select_ll_opcode(MINS_MOVB, size);
        LowLevelOpcode subOpcode = select_ll_opcode(MINS_SUBB, size);

        Operand srcOperand = get_ll_operand(hl_ins->get_operand(1), size, ll_iseq);
        Operand destOperand = get_ll_operand(hl_ins->get_operand(0), size, ll_iseq);

        Operand::Kind kind = select_mreg_kind(size);
        Operand r10(kind, MREG_R10);

        Operand operand0 = Operand(Operand::IMM_IVAL, 0);

        ll_iseq->append(new Instruction(movOpcode, srcOperand, r10));
        ll_iseq->append(new Instruction(movOpcode, operand0, destOperand));
        ll_iseq->append(new Instruction(subOpcode, r10, destOperand));
        return;
    }

    RuntimeError::raise("high level opcode %d not handled", int(hl_opcode));
}

// TODO: implement other private member functions


Operand LowLevelCodeGen::get_ll_operand(Operand hl_opcode, int size,
                                        const std::shared_ptr <InstructionSequence> &ll_iseq) {

    // IMM_IVAL
    if (hl_opcode.is_imm_ival()) {
        Operand operand(Operand::IMM_IVAL, hl_opcode.get_imm_ival());
        return operand;
    }

    // VREG
    if (hl_opcode.get_kind() == Operand::VREG) {
        int numVreg = hl_opcode.get_base_reg();

        // if in m_mregMap, use machine registers
        std::map<int, MachineReg>::iterator iter;
        iter = m_mregMap.find(numVreg);
        if (iter != m_mregMap.end()) {
            // in m_mregMap, use machine registers
            // TODO: use appropriate mreg with correct size
            Operand operand(Operand::MREG64, iter->second);
            return operand;
        }

        // map vreg to memory by numbers
        // rule:
        // vr0: %eax
        // starting from vr10
        // stack offset starting from bottom (negative with the largest abs value)

        // return vr0
        if (numVreg == 0) {
            Operand operand(select_mreg_kind(size), 0);
            return operand;
        }
        // argument vr1-7
        else if (numVreg < 8) {
            // %rdi, %rsi, %rdx, %rcx, %r8, %r9
            // 5, 4, 3, 2, 8, 9
            Operand operand;
            switch (numVreg) {
                case 1:
                    operand = Operand(select_mreg_kind(size), 5);
                    return operand;
                case 2:
                    operand = Operand(select_mreg_kind(size), 4);
                    return operand;
                case 3:
                    operand = Operand(select_mreg_kind(size), 3);
                    return operand;
                case 4:
                    operand = Operand(select_mreg_kind(size), 2);
                    return operand;
                case 5:
                    operand = Operand(select_mreg_kind(size), 8);
                    return operand;
                case 6:
                    operand = Operand(select_mreg_kind(size), 9);
                    return operand;
            }
        }
        else {
            long offset = (numVreg - 10) * 8 - m_total_memory_storage;
            // %rbp: 7
            Operand operand(Operand::MREG64_MEM_OFF, 7, offset);
            return operand;
        }

    }

    // LABEL
    if (hl_opcode.is_label()) {
        if (hl_opcode.get_kind() == Operand::IMM_LABEL) {
            return Operand(Operand::IMM_LABEL, hl_opcode.get_label());
        } else {
            return Operand(Operand::LABEL, hl_opcode.get_label());
        }
    }

    // VREG_MEM
    if (hl_opcode.get_kind() == Operand::VREG_MEM) {
        // steps:
        // movq src to r10
        // mov (r10) to r11
        // return r11

//         NOT TRUE NOW as currently, any (vr15) is associated with a mov, and r10/r11 will not be overlapped in mov, it's OK

        LowLevelOpcode movOpcode1 = MINS_MOVQ;
//        LowLevelOpcode movOpcode2 = select_ll_opcode(MINS_MOVB, size);

        Operand highOperand = Operand(Operand::VREG, hl_opcode.get_base_reg());
        Operand srcOperand = get_ll_operand(highOperand, 8, ll_iseq);

        Operand r11;
        if (m_ifUseRAX) {
            r11 = Operand(Operand::MREG64, MREG_RAX);
            m_ifUseRAX = false;
        } else {
            r11 = Operand(Operand::MREG64, MREG_R11);
            m_ifUseRAX = true;
        }

//        Operand::Kind mregKind11 = select_mreg_kind(size);
//        Operand r11(mregKind11, MREG_R11);
//
//        ll_iseq->append(new Instruction(movOpcode1, srcOperand, r10));
//        ll_iseq->append(new Instruction(movOpcode2, r10.to_memref(), r11));

        ll_iseq->append(new Instruction(movOpcode1, srcOperand, r11));

        return r11.to_memref();
    }

}

//Operand LowLevelCodeGen::nextTempOperand() {
//
//}
