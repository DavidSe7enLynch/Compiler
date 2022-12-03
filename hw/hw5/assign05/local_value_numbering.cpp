//
// Created by Ruirong Huang on 12/2/22.
//

#include "local_value_numbering.h"

// KeyMember

KeyMember::KeyMember(const Operand &operand)
        : m_isValNum(false), m_operand(operand) {
}

KeyMember::KeyMember(std::shared_ptr <ValueNumber> valNum)
        : m_isValNum(true), m_valNum(valNum) {
}

KeyMember::KeyMember() {
}

KeyMember::~KeyMember() {
}

std::shared_ptr<ValueNumber> KeyMember::getValNum() const {
    assert(m_isValNum);
    return m_valNum;
}

const Operand& KeyMember::getOperand() const {
    assert(!m_isValNum);
    return m_operand;
}

bool KeyMember::isValNum() const {
    return m_isValNum;
}

bool KeyMember::isConst() const {
    return false;
}

std::string KeyMember::toStr() {
    // valNum
    if (m_isValNum) return m_valNum->toStr();
    // operand
    return HighLevelFormatter().format_operand(m_operand);
}


// LVNKey

LVNKey::LVNKey(int opcode, KeyMember mem1)
        : m_opcode(opcode), m_numMembers(1), m_members{mem1, KeyMember()} {
}

LVNKey::LVNKey(int opcode, KeyMember mem1, KeyMember mem2)
        : m_opcode(opcode), m_numMembers(2), m_members{mem1, mem2} {
    sortMember();
}

LVNKey::~LVNKey() {
}

unsigned int LVNKey::getNumMembers() const {
    return m_numMembers;
}

int LVNKey::getOpcode() const {
    return m_opcode;
}

KeyMember LVNKey::getMember(unsigned int index) const {
    assert(index < m_numMembers);
    return m_members[index];
}


void LVNKey::sortMember() {
    if (m_numMembers == 1) return;
    // rules:
    // valnum prior than operand
    // lower valnum prior higher valnum
    // lower operand prior higher operand, as long as they have base_reg (no matter if memref)
    // have base_reg prior than non_reg (label, imm)
    if (!m_members[0].isValNum() && m_members[1].isValNum()) {
        // first operand, second valnum
        swapMember();
    } else if ((m_members[0].isValNum() && m_members[1].isValNum()) &&
                (m_members[0].getValNum()->getValNum() > m_members[1].getValNum()->getValNum())) {
        // both valnum, first > second
        swapMember();
    } else if (!m_members[0].isValNum() && !m_members[1].isValNum()) {
        // both operand
        if ((m_members[0].getOperand().has_base_reg() && m_members[1].getOperand().has_base_reg()) &&
                (m_members[0].getOperand().get_base_reg() > m_members[0].getOperand().get_base_reg())) {
            // both have base_reg, first > second
            swapMember();
        } else if (!m_members[0].getOperand().has_base_reg() && m_members[1].getOperand().has_base_reg()) {
            // first doesn't have base_reg, second has
            swapMember();
        }
    }
}

void LVNKey::swapMember() {
    assert(m_numMembers == 2);
    KeyMember temp = m_members[0];
    m_members[0] = m_members[1];
    m_members[1] = temp;
}

std::string LVNKey::toStr() {
    std::string opcode(highlevel_opcode_to_str(HighLevelOpcode(m_opcode)));
    if (m_numMembers == 1)
        return cpputil::format("%s: %s", opcode.c_str(), m_members[0].toStr().c_str());
    else
        return cpputil::format("%s: %s, %s", opcode.c_str(), m_members[0].toStr().c_str(), m_members[1].toStr().c_str());
}


// ValueNumber

ValueNumber::ValueNumber(int valNum, int origVreg, std::shared_ptr <LVNKey> lvnKey)
        : m_valNum(valNum), m_origVreg(origVreg), m_lvnKey(lvnKey) {
}

ValueNumber::~ValueNumber() {
}

int ValueNumber::getValNum() const {
    return m_valNum;
}

int ValueNumber::getOrigVreg() const {
    return m_origVreg;
}

std::shared_ptr <LVNKey> ValueNumber::getLVNKey() const {
    return m_lvnKey;
}

std::string ValueNumber::toStr() {
    return cpputil::format("Val<num: %d, reg: %d>", m_valNum, m_origVreg);
}
