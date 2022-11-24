#include <cassert>
#include "node.h"
#include "symtab.h"
#include "local_storage_allocation.h"
#include "exceptions.h"

LocalStorageAllocation::LocalStorageAllocation()
        : m_total_local_storage(0U), m_next_vreg(VREG_FIRST_LOCAL) {
}

LocalStorageAllocation::~LocalStorageAllocation() {
}

void LocalStorageAllocation::visit_declarator_list(Node *n) {
    visit_children(n);
}

void LocalStorageAllocation::visit_named_declarator(Node *n) {
    std::shared_ptr<Type> type = n->getType();
    Symbol *symbol = n->getSymbol();

    // 5 cases
    // 0: storage in virtual registers: integral / pointer, and its address is never referenced
    // 1: storage in memory
    // 2: global
    // 3: struct field declare: don't allocate memory
    // no longer needed: 4: struct type: allocate mem offset for fields

    if (symbol->isGlobal()) {
        symbol->setStorage(StorageKind::GLOBAL, symbol->get_name());
    } else if (n->getIsMember() && n->getMemberName().rfind("struct ", 0) == 0) {
    } else if ((type->is_pointer() || type->is_integral()) && symbol->getReqStorKind() != StorageKind::MEMORY) {
        symbol->setStorage(StorageKind::VREGISTER, m_next_vreg++);
    }
//    else if (type->is_struct()) {
//        std::printf("struct, name: %s\n", symbol->get_name().c_str());
//        unsigned fieldOffset = m_storage_calc.add_field(type);
//        symbol->setStorage(StorageKind::MEMORY, fieldOffset);
//        StorageCalculator scalc;
//        for (auto i = 0; i < type->get_num_members(); i++) {
//            Member member = type->get_member(i);
//            unsigned fieldOffset = scalc.add_field(member.get_type());
//            member.setOffset(fieldOffset);
//            std::printf("set offset: %d\n", fieldOffset);
//        }
//        Member member = type->get_member(0);
//        std::printf("set offset out: %d\n", member.getOffset());
//    }
    else {
//        std::printf("store in mem: %s\n", symbol->get_name().c_str());
        unsigned fieldOffset = m_storage_calc.add_field(type);
        symbol->setStorage(StorageKind::MEMORY, fieldOffset);
//        std::printf("%s, field0: %d\n", type->as_str().c_str(), type->get_member(0).getOffset());
    }
}

void LocalStorageAllocation::visit_array_declarator(Node *n) {
    // 2 kids
    // 0: named declarator: arr name
    // 1: TOK_INT_LIT: arr size

    // if is a struct field, do not setStorage
    if (n->getIsMember() && n->getMemberName().rfind("struct ", 0) == 0) {}
    else {
        // store the array in memory
        Symbol *symbol = n->getSymbol();
        unsigned offset = m_storage_calc.add_field(n->getType());
        symbol->setStorage(StorageKind::MEMORY, offset);
    }
}

void LocalStorageAllocation::visit_function_definition(Node *n) {
    // 4 kids
    // 1: return type
    // 2: name: TOK_IDENT
    // 3: parameter list
    // 4: statement list

    // need to allocate storage for 0: parameters and 1: local variables

    // 0: parameters
    // store in vr1-vr9
    Node *paramList = n->get_kid(2);
    for (auto i = 0; i < paramList->get_num_kids(); i++) {
        Node *param = paramList->get_kid(i);
        Symbol *symbol = param->getSymbol();
        symbol->setStorage(StorageKind::VREGISTER, i + 1); // start from vr1
        // currently not support more than 9 parameters
        if (i >= 9)
            RuntimeError::raise("more than 9 parameters\n");
    }

    // 1: local variables
    visit(n->get_kid(3));

    // get total storage
    m_storage_calc.finish();
    n->setLocalTotalStorage(m_storage_calc.get_size());
}

void LocalStorageAllocation::visit_function_parameter(Node *n) {
//    std::printf("func param: \n", n->get_loc());
//    SemanticError::raise(n->get_loc(), "should not reach this function\n");
    // do nothing, can only reach this point in function declaration
}

void LocalStorageAllocation::visit_statement_list(Node *n) {
    // store current next_vreg, when leaving the scope, return to this point
    int curVreg = m_next_vreg;
    visit_children(n);
    // need to store next available vreg
    n->setNextVreg(m_next_vreg);
    m_next_vreg = curVreg;
}

void LocalStorageAllocation::visit_literal_value(Node *n) {
    // deal with strings
    // store name here, and struct strStorage in strVector

    LiteralValue val = n->getLiteralValue();
    if (val.get_kind() == LiteralValueKind::STRING) {
        int size = m_strVector.size();
        std::string strName = "_str" + std::to_string(size);
        // need to get raw string
//        std::string strVal = val.get_str_value();
        std::string strVal = val.getStrRaw();
//        std::printf("visit literal: strRaw = %s\n", val.getStrRaw().c_str());
        struct strStorage strStor{strName, strVal};
        m_strVector.push_back(strStor);
//        std::printf("deal with string\n")
        n->setOperand(Operand(Operand::IMM_LABEL, strName));
    }
}

// TODO: implement private member functions
