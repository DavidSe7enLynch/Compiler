#ifndef HIGHLEVEL_FORMATTER_H
#define HIGHLEVEL_FORMATTER_H

#include "formatter.h"
#include "lowlevel_formatter.h"

class HighLevelFormatter : public Formatter {
public:
    HighLevelFormatter();

    virtual ~HighLevelFormatter();

    virtual std::string format_operand(const Operand &operand) const;

    virtual std::string format_instruction(const Instruction *ins) const;

    Operand::Kind select_mreg_kind(int operand_size) const;
};

#endif // HIGHLEVEL_FORMATTER_H
