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

NodeBase::NodeBase() {
}

NodeBase::~NodeBase() {
}

// TODO: implement member functions
void NodeBase::setType(std::shared_ptr<Type> type) {
    typeNode = type;    // allow re-assign
}

std::shared_ptr<Type> NodeBase::getType() {
//    assert(typeNode != nullptr);
    if (symbolNode != nullptr)
        return symbolNode->get_type();
    else {
        assert(typeNode != nullptr);
        return typeNode;
    }
}

void NodeBase::setSymbol(Symbol *symbol) {
    assert(typeNode == nullptr);    // a node cannot have both symbol and type
    assert(symbolNode == nullptr);  // not assigned before
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

void NodeBase::setLiteralKind(LiteralValueKind kind) {
    setIsLiteral(true);
    literalKind = kind;
    if (literalKind == LiteralValueKind::INTEGER) {
        std::shared_ptr<Type> intType(new BasicType(BasicTypeKind::INT, true));
        typeNode = intType;
    } else if (literalKind == LiteralValueKind::CHARACTER) {
        std::shared_ptr<Type> charType(new BasicType(BasicTypeKind::CHAR, true));
        typeNode = charType;
    } else if (literalKind == LiteralValueKind::STRING) {
        std::shared_ptr<Type> charType(new BasicType(BasicTypeKind::CHAR, true));
        std::shared_ptr<Type> ptrType(new PointerType(charType));
        typeNode = ptrType;
    }
}

LiteralValueKind NodeBase::getLiteralKind() {
    assert(isLiteral);
    return literalKind;
}

//void NodeBase::setIsInFunc(bool isinfunc) {
//    isInFunc = isinfunc;
//}
//
//bool NodeBase::getIsInFunc() {
//    return isInFunc;
//}

void NodeBase::setFuncName(std::string funcname) {
    isInFunc = true;
    funcName = funcname;
}

std::string NodeBase::getFuncName() {
    assert(isInFunc);
    return funcName;
}

void NodeBase::setIsArray(bool isarray) {
    isArray = isarray;
}

bool NodeBase::getIsArray() {
    return isArray;
}
