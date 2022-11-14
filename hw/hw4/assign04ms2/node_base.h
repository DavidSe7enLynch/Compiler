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

#ifndef NODE_BASE_H
#define NODE_BASE_H

#include <memory>
#include "type.h"
#include "symtab.h"
#include "literal_value.h"
#include "operand.h"

// The Node class will inherit from this type, so you can use it
// to define any attributes and methods that Node objects should have
// (constant value, results of semantic analysis, code generation info,
// etc.)
class NodeBase {
private:
    // TODO: fields (pointer to Type, pointer to Symbol, etc.)
    std::shared_ptr <Type> typeNode;
    Symbol *symbolNode;

    bool isMember;
    std::string memberName;

    bool isLiteral;
//    LiteralValueKind literalKind;
    LiteralValue m_literalValue;

    bool isInFunc;
//    std::string funcName;
    Symbol *funcSymbol;

    // only useful for array element ref node
    // for 2-D array to check if it contains an array
    bool isArray;

    // only useful for statement list node
    int m_nextVreg;
    // only useful for function definition node
    unsigned m_localTotalStorage;

    Operand m_operand;

    // copy ctor and assignment operator not supported
    NodeBase(const NodeBase &);

    NodeBase &operator=(const NodeBase &);

public:
    NodeBase();

    virtual ~NodeBase();

    void setType(std::shared_ptr <Type> type);

    std::shared_ptr <Type> getType();

    void setSymbol(Symbol *symbol);

    Symbol *getSymbol();

    bool hasSymbol();

    void setMemberName(std::string name);

    std::string getMemberName();

    void setIsMember(bool ismember);

    bool getIsMember();

    void setIsLiteral(bool isliteral);

    bool getIsLiteral();

//    void setLiteralKind(LiteralValueKind kind);

    LiteralValueKind getLiteralKind();

//    void setIsInFunc(bool isinfunc);
//    bool getIsInFunc();
//    void setFuncName(std::string funcname);

//    std::string getFuncName();

    void setFuncSymbol(Symbol *symbol);

    Symbol *getFuncSymbol();

    void setIsArray(bool isarray);

    bool getIsArray();

    void setNextVreg(int nextVreg);

    int getNextVreg() const;

    void setLocalTotalStorage(unsigned localTotalStorage);

    unsigned getLocalTotalStorage() const;

    void setOperand(const Operand &operand);

    const Operand &getOperand() const;

    void setLiteralValue(const LiteralValue &other);

    LiteralValue &getLiteralValue();
};

#endif // NODE_BASE_H
