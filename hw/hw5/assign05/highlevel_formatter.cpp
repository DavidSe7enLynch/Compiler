#include <stdexcept>
#include "cpputil.h"
#include "instruction.h"
#include "highlevel.h"
#include "highlevel_formatter.h"
#include <cassert>

HighLevelFormatter::HighLevelFormatter() {
}

HighLevelFormatter::~HighLevelFormatter() {
}

std::string HighLevelFormatter::format_operand(const Operand &operand) const {
    if (!operand.hasMreg() && !operand.hasMemAddr()) {
        // no allocated mreg or mem addr
        switch (operand.get_kind()) {
            case Operand::VREG:
                return cpputil::format("vr%d", operand.get_base_reg());
            case Operand::VREG_MEM:
                return cpputil::format("(vr%d)", operand.get_base_reg());
            case Operand::VREG_MEM_IDX:
                return cpputil::format("(vr%d, vt%d)", operand.get_base_reg(), operand.get_index_reg());
            case Operand::VREG_MEM_OFF:
                return cpputil::format("%ld(vr%dq)", operand.get_imm_ival(), operand.get_base_reg());
            default:
                return Formatter::format_operand(operand);
        }
    } else {
        Operand mregOperand;
        if (operand.hasMreg()) {
            // has allocated mreg
            int mreg = operand.getMreg().first;
            int size = operand.getMreg().second;
//        std::printf("format_operand, hasMreg%d, mreg%d, size%d, vreg%d\n", operand.hasMreg(), mreg, size, operand.get_base_reg());
            Operand::Kind kind = select_mreg_kind(size);
            mregOperand = Operand(kind, mreg);
        } else {
            // has allocated mem addr
            long memAddr = operand.getMemAddr();
            // %rbp: 7
            mregOperand = Operand(Operand::MREG64_MEM_OFF, 7, memAddr);
        }
        std::string mregStr = LowLevelFormatter().format_operand(mregOperand);
        switch (operand.get_kind()) {
            case Operand::VREG:
                return cpputil::format("vr%d<%s>", operand.get_base_reg(), mregStr.c_str());
            case Operand::VREG_MEM:
                return cpputil::format("(vr%d<%s>)", operand.get_base_reg(), mregStr.c_str());
            case Operand::VREG_MEM_IDX:
                return cpputil::format("(vr%d<%s>, vt%d)", operand.get_base_reg(), mregStr.c_str(), operand.get_index_reg());
            case Operand::VREG_MEM_OFF:
                return cpputil::format("%ld(vr%dq<%s>)", operand.get_imm_ival(), operand.get_base_reg(), mregStr.c_str());
            default:
                return Formatter::format_operand(operand);
        }
    }
}

std::string HighLevelFormatter::format_instruction(const Instruction *ins) const {
    HighLevelOpcode opcode = HighLevelOpcode(ins->get_opcode());

    const char *mnemonic_ptr = highlevel_opcode_to_str(opcode);
    if (mnemonic_ptr == nullptr) {
        std::string exmsg = cpputil::format("Unknown highlevel opcode: %d", ins->get_opcode());
        throw std::runtime_error(exmsg);
    }
    std::string mnemonic(mnemonic_ptr);

    std::string buf;

    buf += mnemonic;
    // pad mnemonics to 8 characters
    unsigned padding = (mnemonic.size() < 8U) ? 8U - mnemonic.size() : 0;
    buf += ("         " + (8U - padding));
    for (unsigned i = 0; i < ins->get_num_operands(); i++) {
        if (i > 0) {
            buf += ", ";
        }
        buf += format_operand(ins->get_operand(i));
    }

    return buf;
}

// Get the correct Operand::Kind value for a machine register
// of the specified size (1, 2, 4, or 8 bytes.)
Operand::Kind HighLevelFormatter::select_mreg_kind(int operand_size) const {
//    std::printf("select_mreg_kind: size = %d\n", operand_size);
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
    }
}
