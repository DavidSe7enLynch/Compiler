// Copyright (c) 2021, David H. Hovemeyer <david.hovemeyer@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <cassert>
#include "node_base.h"

NodeBase::NodeBase() : m_nextVreg(0), m_localTotalStorage(0) {
}

NodeBase::~NodeBase() {
}

// TODO: implement member functions
void NodeBase::setType(std::shared_ptr <Type> type) {
    typeNode = type;    // allow re-assign
}

std::shared_ptr <Type> NodeBase::getType() {
//    assert(typeNode != nullptr);
    // if the node has both symbol and type, they can be different, for arrays
    // array symbol, but base/pointer type
    if (typeNode != nullptr)
        return typeNode;
    else if (symbolNode != nullptr)
        return symbolNode->get_type();
}

void NodeBase::setSymbol(Symbol *symbol) {
//    assert(typeNode == nullptr);    // a node cannot have both symbol and type
//    assert(symbolNode == nullptr);  // not assigned before
    symbolNode = symbol;
}

Symbol *NodeBase::getSymbol() {
    assert(symbolNode != nullptr);
    return symbolNode;
}

bool NodeBase::hasSymbol() {
    if (symbolNode != nullptr)
        return true;
    return false;
}

void NodeBase::setMemberName(std::string name) {
    memberName = name;
}

std::string NodeBase::getMemberName() {
    return memberName;
}

void NodeBase::setIsMember(bool ismember) {
    isMember = ismember;
}

bool NodeBase::getIsMember() {
    return isMember;
}

void NodeBase::setIsLiteral(bool isliteral) {
    isLiteral = isliteral;
}

bool NodeBase::getIsLiteral() {
    return isLiteral;
}

//void NodeBase::setLiteralKind(LiteralValueKind kind) {
//
//}

LiteralValueKind NodeBase::getLiteralKind() {
    assert(isLiteral);
    return m_literalValue.get_kind();
}

//void NodeBase::setIsInFunc(bool isinfunc) {
//    isInFunc = isinfunc;
//}
//
//bool NodeBase::getIsInFunc() {
//    return isInFunc;
//}

//void NodeBase::setFuncName(std::string funcname) {
//    isInFunc = true;
//    funcName = funcname;
//}
//
//std::string NodeBase::getFuncName() {
//    assert(isInFunc);
//    return funcName;
//}

void NodeBase::setFuncSymbol(Symbol *symbol) {
    isInFunc = true;
    funcSymbol = symbol;
}

Symbol *NodeBase::getFuncSymbol() {
    assert(isInFunc);
    return funcSymbol;
}

void NodeBase::setIsArray(bool isarray) {
    isArray = isarray;
}

bool NodeBase::getIsArray() {
    return isArray;
}

void NodeBase::setNextVreg(int nextVreg) {
    m_nextVreg = nextVreg;
}

int NodeBase::getNextVreg() const {
    assert(m_nextVreg != 0);
    return m_nextVreg;
}

void NodeBase::setLocalTotalStorage(unsigned int localTotalStorage) {
    m_localTotalStorage = localTotalStorage;
}

unsigned NodeBase::getLocalTotalStorage() const {
    return m_localTotalStorage;
}

void NodeBase::setOperand(const Operand &operand) {
    m_operand = operand;
}

const Operand &NodeBase::getOperand() const {
    return m_operand;
}

void NodeBase::setLiteralValue(const LiteralValue &other) {
    m_literalValue = other;
//    std::printf("node set literal, strRaw = %s\n", m_literalValue.getStrRaw().c_str());
    setIsLiteral(true);
    LiteralValueKind literalKind = m_literalValue.get_kind();
    if (literalKind == LiteralValueKind::INTEGER) {
        // int, long
        if (m_literalValue.is_long()) {
//            std::printf("setlitval, is long\n");
            std::shared_ptr <Type> longType(new BasicType(BasicTypeKind::LONG, true));
            typeNode = longType;
        } else {
            std::shared_ptr <Type> intType(new BasicType(BasicTypeKind::INT, true));
            typeNode = intType;
        }
    } else if (literalKind == LiteralValueKind::CHARACTER) {
        std::shared_ptr <Type> charType(new BasicType(BasicTypeKind::CHAR, true));
        typeNode = charType;
    } else if (literalKind == LiteralValueKind::STRING) {
        std::shared_ptr <Type> charType(new BasicType(BasicTypeKind::CHAR, true));
        std::shared_ptr <Type> ptrType(new PointerType(charType));
        typeNode = ptrType;
    }
}

LiteralValue &NodeBase::getLiteralValue() {
    return m_literalValue;
}

void NodeBase::setVarName(std::string varname) {
    varName = varname;
}

std::string NodeBase::getVarName() {
    return varName;
}
