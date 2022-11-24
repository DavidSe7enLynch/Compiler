#ifndef LOCAL_STORAGE_ALLOCATION_H
#define LOCAL_STORAGE_ALLOCATION_H

#include "storage.h"
#include "ast_visitor.h"

class LocalStorageAllocation : public ASTVisitor {
public:
    // vr0 is the return value vreg
    static const int VREG_RETVAL = 0;

    // vr1 is 1st argument vreg
    static const int VREG_FIRST_ARG = 1;

    // local variable allocation starts at vr10
    static const int VREG_FIRST_LOCAL = 10;

    struct strStorage {
        std::string name;
        std::string val;
    };

    std::vector<strStorage> m_strVector;

private:
    StorageCalculator m_storage_calc;
    unsigned m_total_local_storage;
    int m_next_vreg;

public:
    LocalStorageAllocation();

    virtual ~LocalStorageAllocation();

    virtual void visit_declarator_list(Node *n);

//    virtual void visit_function_declaration(Node *n);

    virtual void visit_function_definition(Node *n);

    virtual void visit_function_parameter(Node *n);

    virtual void visit_statement_list(Node *n);

    virtual void visit_named_declarator(Node *n);

    virtual void visit_array_declarator(Node *n);

    virtual void visit_literal_value(Node *n);

private:
    // TODO: add private member functions
};

#endif // LOCAL_STORAGE_ALLOCATION_H
