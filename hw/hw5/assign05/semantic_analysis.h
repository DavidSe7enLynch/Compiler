#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include <cstdint>
#include <memory>
#include <utility>
#include "type.h"
#include "symtab.h"
#include "ast_visitor.h"
#include "literal_value.h"

class Node;
//class LiteralValue;

class SemanticAnalysis : public ASTVisitor {
private:
    SymbolTable *m_global_symtab, *m_cur_symtab;

public:
    SemanticAnalysis();

    virtual ~SemanticAnalysis();

    SymbolTable *get_global_symtab();

    virtual void visit_struct_type(Node *n);

    virtual void visit_union_type(Node *n);

    virtual void visit_variable_declaration(Node *n);

    virtual void visit_basic_type(Node *n);

    virtual void visit_function_definition(Node *n);

    virtual void visit_function_declaration(Node *n);

    virtual void visit_function_parameter(Node *n);

    virtual void visit_statement_list(Node *n);

    virtual void visit_struct_type_definition(Node *n);

    virtual void visit_binary_expression(Node *n);

    virtual void visit_unary_expression(Node *n);

    virtual void visit_postfix_expression(Node *n);

    virtual void visit_conditional_expression(Node *n);

    virtual void visit_cast_expression(Node *n);

    virtual void visit_function_call_expression(Node *n);

    virtual void visit_field_ref_expression(Node *n);

    virtual void visit_indirect_field_ref_expression(Node *n);

    virtual void visit_array_element_ref_expression(Node *n);

    virtual void visit_variable_ref(Node *n);

    virtual void visit_literal_value(Node *n);

    virtual void visit_pointer_declarator(Node *n);

    virtual void visit_array_declarator(Node *n);

    virtual void visit_named_declarator(Node *n);

    virtual void visit_field_definition_list(Node *n);

    virtual void visit_return_expression_statement(Node *n);

private:
    // TODO: add helper functions

    void visit_tokChar(Node *n);

    void visit_tokVoid(Node *n);

    void visit_tokInt(Node *n);

    void visit_tokShort(Node *n);

    void visit_tokLong(Node *n);

    void enterScope();

    void leaveScope();

    void passMember(Node *n);

    void callAddMember(Node *n);

    void basicDefAnalysis(Node *n, int array[]);

    void visit_Assign(Node *n);

//    void analyzeAssignLiteral(std::shared_ptr<Type> ltype, LiteralValueKind literalKind, Node *n);

    void analyzeAssignRef(std::shared_ptr <Type> ltype, std::shared_ptr <Type> rtype, Node *n);

    void isLvalue(Node *n);

    void passFuncSymbAllNodes(Node *n);

    void passDeclareTypeAllNodes(Node *n, std::shared_ptr<Type> type);

//    Node *promote_to_int(Node *n);

    Node *implicit_conversion(Node *n, const std::shared_ptr<Type> &type);

    void binaryConvCheck(Node *n);
};

#endif // SEMANTIC_ANALYSIS_H
