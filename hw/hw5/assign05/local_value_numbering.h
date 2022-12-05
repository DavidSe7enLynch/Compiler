//
// Created by Ruirong Huang on 12/2/22.
//

#ifndef HW5_LOCAL_VALUE_NUMBERING_H
#define HW5_LOCAL_VALUE_NUMBERING_H

#include <memory>
#include "cpputil.h"
#include "operand.h"
#include "highlevel_formatter.h"
#include "highlevel.h"

class ValueNumber;

class LVNKey;

//typedef union {
//    const Operand &operand;
//    std::shared_ptr<ValueNumber> valNum;
//} KeyMember;

class KeyMember {
private:
    Operand m_operand;
    std::shared_ptr <ValueNumber> m_valNum;
    bool m_isValNum;

public:
    KeyMember(const Operand &operand);

    KeyMember(std::shared_ptr <ValueNumber> valNum);

    KeyMember(); // null

    ~KeyMember();

    bool isValNum() const;

    std::shared_ptr <ValueNumber> getValNum() const;

    const Operand &getOperand() const;

    std::string toStr();

    bool isSame(KeyMember other);

    bool isConstNum();

    long getConstNum();
};




class LVNKey {
private:
    int m_opcode;
    unsigned m_numMembers;
    KeyMember m_members[2];
    bool m_isConstNum;
    long m_constNum;

public:
//    LVNKey(int ); // no dependency, m_opcode =

    LVNKey(int opcode, KeyMember mem1);

    LVNKey(int opcode, KeyMember mem1, KeyMember mem2);

    ~LVNKey();

    int getOpcode() const;

    unsigned getNumMembers() const;

    KeyMember getMember(unsigned index) const;

    std::string toStr();

    bool isSame(std::shared_ptr<LVNKey> other);

    bool isConstNum();

    long getConstNum();

private:
    void sortMember();
    void swapMember();
};


class ValueNumber {
private:
    int m_valNum;
    int m_origVreg;
    std::shared_ptr <LVNKey> m_lvnKey;

public:
    ValueNumber(int valNum, int origVreg, std::shared_ptr <LVNKey> lvnKey);

    ~ValueNumber();

    int getValNum() const;

    int getOrigVreg() const;

    std::shared_ptr <LVNKey> getLVNKey() const;

    std::string toStr();

    bool isConstNum();

    long getConstNum();
};


#endif //HW5_LOCAL_VALUE_NUMBERING_H
