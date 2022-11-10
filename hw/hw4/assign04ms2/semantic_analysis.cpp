#include <cassert>
#include <algorithm>
#include <utility>
#include <map>
#include "grammar_symbols.h"
#include "parse.tab.h"
#include "node.h"
#include "ast.h"
#include "exceptions.h"
#include "semantic_analysis.h"

SemanticAnalysis::SemanticAnalysis()
        : m_global_symtab(new SymbolTable(nullptr)) {
    m_cur_symtab = m_global_symtab;
}

SemanticAnalysis::~SemanticAnalysis() {
}

SymbolTable *SemanticAnalysis::get_global_symtab() {
    return m_global_symtab;
}

void SemanticAnalysis::visit_struct_type(Node *n) {
    std::string nameStruct = "struct " + n->get_kid(0)->get_str();  // get name

    Symbol *symbolStruct = m_cur_symtab->lookup_recursive(nameStruct);
    if (symbolStruct == nullptr)
        SemanticError::raise(n->get_loc(), "symbolStruct doesn't exist\n");

    n->setType(symbolStruct->get_type());

//    std::printf("in visit_struct_type, isMember = %d, memberName = %s\n", n->getIsMember(), n->getMemberName().c_str());
}

void SemanticAnalysis::visit_union_type(Node *n) {
    RuntimeError::raise("union types aren't supported");
}

void SemanticAnalysis::visit_variable_declaration(Node *n) {
    // 3 kids
    // second: type (all kinds of)
    // third: literals
    visit(n->get_kid(1));
    std::shared_ptr<Type> type = n->get_kid(1)->getType();

    passMember(n);
    passMember(n->get_kid(2));

    // set type for variables
    for (auto i = 0; i < n->get_kid(2)->get_num_kids(); i++) {
        Node *kid = n->get_kid(2)->get_kid(i);
        kid->setType(type);
        visit(kid);
    }
}

void SemanticAnalysis::visit_basic_type(Node *n) {
//    std::printf("in visit basic type: %d\n", n->get_num_kids());
//    bool isUnsigned = false;
//    bool isSigned = false;
//    bool isNum = false;
//    bool isConst = false;
//    bool isVolatile = false;
//    bool isChar = false;
//    bool isVoid = false;
//    bool isLong = false;
//    bool isShort = false;
//    bool isInt = false;

    // a better way, an array
    int array[9] = {0};
    // 0: void
    // 1: char
    // 2: int
    // 3: long
    // 4: short
    // 5: signed
    // 6: unsigned
    // 7: const
    // 8: volatile

    for (auto i = 0; i < n->get_num_kids(); i++) {
        Node *kid = n->get_kid(i);
        int tag = kid->get_tag();
//        std::printf("tok tag: %d", tag);
        switch (tag) {
            case TOK_VOID:
                array[0] = 1;
                break;
            case TOK_CHAR:
                array[1] = 1;
                break;
            case TOK_INT:
                array[2] = 1;
                break;
            case TOK_LONG:
                array[3] = 1;
                break;
            case TOK_SHORT:
                array[4] = 1;
                break;
            case TOK_SIGNED:
                array[5] = 1;
                break;
            case TOK_UNSIGNED:
                array[6] = 1;
                break;
            case TOK_CONST:
                array[7] = 1;
                break;
            case TOK_VOLATILE:
                array[8] = 1;
                break;
            default:
                SemanticError::raise(n->get_loc(), "wrong type in basic type\n");
        }
    }

    // after reading all types, do semantic analysis
    basicDefAnalysis(n, array);
}


void SemanticAnalysis::visit_function_definition(Node *n) {
    // 4 kids
    // 1: return type
    // 2: name: TOK_IDENT
    // 3: parameter list
    // 4: statement list

    // steps:
    // get return type
    // get name
    // create func type and symbol
    // in a deeper scope, get parameter types, add_member to func type
    // in a deeper deeper scope, get statement types
    // pass function name to return expression to check return type
    // back to outer scope


    visit(n->get_kid(0));
    std::shared_ptr<Type> returnType = n->get_kid(0)->getType();

    std::string name = n->get_kid(1)->get_str();

    // if func declared but not defined, revisit param list, but do not add_symbol
    Symbol *funcSymbol = m_cur_symtab->lookup_recursive(name);
    if (funcSymbol != nullptr && !funcSymbol->is_defined()) {
        n->setType(funcSymbol->get_type());
        funcSymbol->set_is_defined(true);    // define it

        // revisit param list, but do not add_symbol
        enterScope();
        // inside param list
        Node *paramList = n->get_kid(2);
        visit(paramList);

        // go to statement list, pass func name to statements
        Node *stateList = n->get_kid(3);
        stateList->setFuncSymbol(funcSymbol);
        passFuncSymbAllNodes(stateList);

        // go to statement list
        visit(stateList);
        leaveScope();
    } else if (funcSymbol != nullptr) {
        // name already used by non-function type or defined function type
        if (m_cur_symtab->has_symbol_local(name))
            SemanticError::raise(n->get_loc(), "symbol name already used\n");
    } else {
        // if func not declared hrr

        // func type
        std::shared_ptr<FunctionType> functionType(new FunctionType(returnType));
        n->setType(functionType);
        // symbol
        m_cur_symtab->define(SymbolKind::FUNCTION, name, functionType);
        Symbol *funcSymbol = m_cur_symtab->lookup_recursive(name);
        assert(funcSymbol->get_type()->is_function());

        enterScope();
        // inside param list
        Node *paramList = n->get_kid(2);
        // initally set membership
        paramList->setIsMember(true);
        paramList->setMemberName(name);
        passMember(paramList);
        // now each parameter node has membership
        // we need to add each of them to symbol table, and add_member to func

        visit(paramList);

        // go to statement list, pass func name to statements
        Node *stateList = n->get_kid(3);
        stateList->setFuncSymbol(funcSymbol);
        passFuncSymbAllNodes(stateList);
        visit(stateList);

        leaveScope();
    }
//    Symbol *symbol = m_cur_symtab->lookup_recursive(name);
//    std::printf("after function definition, func symbol = (name: %s, type: %s)\n", symbol->get_name().c_str(),
//                symbol->get_type()->as_str().c_str());
}

void SemanticAnalysis::visit_function_declaration(Node *n) {
    // 3 kids
    // 1: return type
    // 2: name: TOK_IDENT
    // 3: parameter list

    // steps:
    // get return type
    // get name
    // create func type and symbol
    // in a deeper scope, get parameter types, add_member to func type
    // back to outer scope

    visit(n->get_kid(0));
    std::shared_ptr<Type> returnType = n->get_kid(0)->getType();

    std::string name = n->get_kid(1)->get_str();
    // make sure name is not used in current scope
    if (m_cur_symtab->has_symbol_local(name))
        SemanticError::raise(n->get_loc(), "symbol name already used\n");

    // func type
    std::shared_ptr<FunctionType> functionType(new FunctionType(returnType));
    n->setType(functionType);
    // symbol
    m_cur_symtab->declare(SymbolKind::FUNCTION, name, functionType);

    enterScope();
    // inside param list
    Node *paramList = n->get_kid(2);
    // initally set membership
    paramList->setIsMember(true);
    paramList->setMemberName(name);
    passMember(paramList);
    // now each parameter node has membership
    // we need to add each of them to symbol table, and add_member to func

    visit(paramList);
    leaveScope();

    Symbol *symbol = m_cur_symtab->lookup_recursive(name);
//    std::printf("after function declaration, func symbol = (name: %s, type: %s)\n", symbol->get_name().c_str(),
//                symbol->get_type()->as_str().c_str());
}

void SemanticAnalysis::visit_function_parameter(Node *n) {
    // 2 kids
    // 1: type (all kinds of)
    // 2: 1 literal
//    std::printf("in function parameter\n");
    visit(n->get_kid(0));
    std::shared_ptr<Type> type = n->get_kid(0)->getType();
//    std::printf("in function parameter2\n");

    passMember(n);

    // set type for the variable
    Node *kid = n->get_kid(1);
    kid->setType(type);
    visit(kid);
    n->setSymbol(kid->getSymbol());
//    std::printf("sematic analysis visiting function parameter, symbol: %s\n", n->getSymbol()->get_name().c_str());
}

void SemanticAnalysis::visit_statement_list(Node *n) {
    enterScope();
    visit_children(n);
    leaveScope();
}

void SemanticAnalysis::visit_struct_type_definition(Node *n) {
    // add symbol
    std::string name = n->get_kid(0)->get_str();
    // make sure name is not used
//    std::printf("name: %s, if exist: %d\n", name.c_str(), m_cur_symtab->has_symbol_local(name));
    if (m_cur_symtab->has_symbol_local("struct " + name))
        SemanticError::raise(n->get_loc(), "symbol name already used\n");

    std::shared_ptr<StructType> structType(new StructType(name));
    m_cur_symtab->define(SymbolKind::TYPE, "struct " + name, structType);

    // initial set membership for later passing
    n->get_kid(1)->setIsMember(true);
    n->get_kid(1)->setMemberName("struct " + name);

    // fields
    enterScope();
    visit(n->get_kid(1));
    leaveScope();

}

void SemanticAnalysis::visit_binary_expression(Node *n) {
    // =, -, +, *, /, %, ...
    // 3 kids
    // 0: operator
    // 1: left operand
    // 2: right operand
    int tag = n->get_kid(0)->get_tag();
    Node *lnode = n->get_kid(1);
    Node *rnode = n->get_kid(2);
    visit(lnode);
    visit(rnode);
    std::shared_ptr<Type> ltype = lnode->getType();
    std::shared_ptr<Type> rtype = rnode->getType();
//    std::printf("binary, ltype: %s, rtype: %s\n", ltype->as_str().c_str(), rtype->as_str().c_str());

    // assign
    if (tag == TOK_ASSIGN) {
        visit_Assign(n);
        return;
    }

    // rules
    // (literal value) integer, (literal value) integer: good
    // pointer, pointer: bad
    // +, -: pointer, integer: good

    if (ltype->is_integral() && rtype->is_integral()) {
        // good
        // check implicit conversion
        binaryConvCheck(n);
        n->setType(ltype);
    } else if (ltype->is_pointer() && rtype->is_pointer()) {
        // bad
        SemanticError::raise(n->get_loc(), "binary: two pointers\n");
    } else if ((tag == TOK_PLUS || tag == TOK_MINUS) &&
               ((ltype->is_pointer() && rtype->is_integral() || (rtype->is_pointer() && ltype->is_integral())))) {
        // good
        if (ltype->is_pointer())
            n->setType(ltype);
        else
            n->setType(rtype);
    } else if ((tag == TOK_EQUALITY || tag == TOK_LOGICAL_AND || tag == TOK_LOGICAL_OR)
               && (ltype->is_same(rtype.get()))) {
        // good
        std::shared_ptr<Type> intType(new BasicType(BasicTypeKind::INT, true));
        n->setType(intType);
    } else {
        SemanticError::raise(n->get_loc(), "binary: wrong\n");
    }
}

void SemanticAnalysis::binaryConvCheck(Node *n) {
//    std::printf("in conv check\n");
    Node *lnode = n->get_kid(1);
    Node *rnode = n->get_kid(2);
    std::shared_ptr<Type> ltype = lnode->getType();
    std::shared_ptr<Type> rtype = rnode->getType();

    // check implicit conversion
    // 3 rules
    // 0: if either operand is less precise than int or unsigned int, promote it to int or unsigned int
    // 1: if one is less precise than the other, promote it to the other
    // 2: signed convert to unsigned

    bool isConvertSigned = true;
    // if at leaset one is unsigned, set to unsigned
    if (!ltype->is_signed() || !rtype->is_signed()) {
        isConvertSigned = false;
    }
    // if both less precise than int
    if (ltype->get_basic_type_kind() < BasicTypeKind::INT && rtype->get_basic_type_kind() < BasicTypeKind::INT) {
        std::shared_ptr<Type> type(new BasicType(BasicTypeKind::INT, isConvertSigned));
        n->set_kid(1, implicit_conversion(lnode, type));
        n->set_kid(2, implicit_conversion(rnode, type));
    }
        // if ltype is less precise than rtype, or rtype is less precise than ltype
    else if (ltype->get_basic_type_kind() < rtype->get_basic_type_kind()) {
        // both signed, or rtype unsigned, no problem
        if (!rtype->is_signed() || (ltype->is_signed() && rtype->is_signed())) {
            n->set_kid(1, implicit_conversion(lnode, rtype));
        }
            // ltype unsigned, rtype signed,
            // lnode convert to more precise unsigned rtype
            // rnode convert to unsigned
        else if (!ltype->is_signed() && rtype->is_signed()) {
            std::shared_ptr<Type> type(new BasicType(rtype->get_basic_type_kind(), false));
            n->set_kid(1, implicit_conversion(lnode, type));
            n->set_kid(2, implicit_conversion(rnode, type));
        }
    } else if (rtype->get_basic_type_kind() < ltype->get_basic_type_kind()) {
        // both signed, or ltype unsigned, no problem
        if (!ltype->is_signed() || (ltype->is_signed() && rtype->is_signed())) {
            n->set_kid(2, implicit_conversion(rnode, ltype));
        }
            // rtype unsigned, ltype signed,
            // rnode convert to more precise unsigned rtype
            // lnode convert to unsigned
        else if (!rtype->is_signed() && ltype->is_signed()) {
            std::shared_ptr<Type> type(new BasicType(ltype->get_basic_type_kind(), false));
            n->set_kid(1, implicit_conversion(lnode, type));
            n->set_kid(2, implicit_conversion(rnode, type));
        }
    }
        // same precision, but different signed
    else if (ltype->is_signed() && !rtype->is_signed()) {
        n->set_kid(1, implicit_conversion(lnode, rtype));
    } else if (rtype->is_signed() && !ltype->is_signed()) {
        n->set_kid(2, implicit_conversion(rnode, ltype));
    }
}

void SemanticAnalysis::visit_unary_expression(Node *n) {
    // var address
    // minus
    // pointer deref
    // !

    Node *op = n->get_kid(0);
    Node *val = n->get_kid(1);
    int tag = op->get_tag();
    visit(val);

    if (val->getIsLiteral() && tag != TOK_MINUS)
        SemanticError::raise(n->get_loc(), "unary: operand should not have literal kind\n");

    // check implicit conversion
    // if the operand is less precise than int or unsigned int, promote it to int or unsigned int
    if (val->getType()->is_integral() && val->getType()->get_basic_type_kind() < BasicTypeKind::INT) {
        std::shared_ptr<Type> type(new BasicType(BasicTypeKind::INT, val->getType()->is_signed()));
        n->set_kid(1, implicit_conversion(val, type));
    }

    switch (tag) {
        case TOK_ASTERISK: {
            // *
            if (val->getType()->is_pointer())
                n->setType(val->getType()->get_base_type());
            else
                SemanticError::raise(n->get_loc(), "unary: value of non-pointer\n");
            break;
        }
        case TOK_MINUS: {
            if (val->getIsLiteral() && val->getLiteralKind() == LiteralValueKind::INTEGER) {
                std::shared_ptr<Type> intType(new BasicType(BasicTypeKind::INT, true));
                n->setType(intType);
                break;
            } else if (val->getType()->is_integral()) {
                n->setType(val->getType());
                break;
            } else
                SemanticError::raise(n->get_loc(), "unary: minus non-integral\n");
        }
        case TOK_AMPERSAND: {
            // &
            // need to indicate that the referenced symbol should be stored in memory
            if (val->hasSymbol())
                val->getSymbol()->setReqStorKind(StorageKind::MEMORY);
//            std::printf("in unary tok_&, val->type: %s\n", val->getType()->as_str().c_str());
            std::shared_ptr<Type> pointerType(new PointerType(val->getType()));
            n->setType(pointerType);
            break;
        }
        case TOK_NOT: {
            n->setType(val->getType());
            break;
        }
        default:
            SemanticError::raise(n->get_loc(), "wrong unary operator\n");
    }
//    std::printf("unary: type: %s\n", n->getType()->as_str().c_str());
}

void SemanticAnalysis::visit_postfix_expression(Node *n) {
    // TODO: implement
}

void SemanticAnalysis::visit_conditional_expression(Node *n) {
    // TODO: implement
}

void SemanticAnalysis::visit_cast_expression(Node *n) {
    // TODO: implement
}

void SemanticAnalysis::visit_function_call_expression(Node *n) {
    // 2 kids
    // 0: variable ref: function name
    // 1: argument list

    // make sure function is declared
    std::string funcName = n->get_kid(0)->get_kid(0)->get_str();
    Symbol *symbol = m_cur_symtab->lookup_recursive(funcName);
    if (symbol == nullptr)
        SemanticError::raise(n->get_loc(), "function call: function not declared\n");

    // num of parameters is correct
    Node *paramList = n->get_kid(1);
    int numParam = symbol->get_type()->get_num_members();
    if (numParam != paramList->get_num_kids())
        SemanticError::raise(n->get_loc(), "function call: wrong num of params\n");

    // each param has correct type
    for (auto i = 0; i < numParam; i++) {
        Node *param = paramList->get_kid(i);
        visit(param);
//        std::printf("in params, param type: %s\n", param->getType()->as_str().c_str());
        analyzeAssignRef(symbol->get_type()->get_member(i).get_type(), param->getType(), n);
    }
//    std::printf("function call, return type: %s\n", symbol->get_type()->get_base_type()->as_str().c_str());
    n->setType(symbol->get_type()->get_base_type());
}

void SemanticAnalysis::visit_field_ref_expression(Node *n) {
    // 2 kids
    // 0: var ref of struct name
    // tok_ident of field name

    visit(n->get_kid(0));
    std::shared_ptr<Type> structType = n->get_kid(0)->getType();
    std::string fieldName = n->get_kid(1)->get_str();

    // make sure it is a struct type
    if (!structType->is_struct())
        SemanticError::raise(n->get_loc(), "bad field ref of non-struct type\n");

    // search for this field
    for (auto i = 0; i < structType->get_num_members(); i++) {
        Member member = structType->get_member(i);
        if (member.get_name() == fieldName) {
            std::shared_ptr<Type> type = member.get_type();
            // handle array as pointer
            if (type->is_array()) {
                std::shared_ptr<Type> ptrType(new PointerType(type->get_base_type()));
                n->setType(ptrType);
            } else
                n->setType(type);
            return;
        }
    }
    // wrong field name
    SemanticError::raise(n->get_loc(), "wrong struct field name\n");
}

void SemanticAnalysis::visit_indirect_field_ref_expression(Node *n) {
    // 2 kids
    // 0: var ref of struct name
    // tok_ident of field name
    visit(n->get_kid(0));
    std::shared_ptr<Type> structPtrType = n->get_kid(0)->getType();
    std::string fieldName = n->get_kid(1)->get_str();

    // make sure it is a struct type
    if (!(structPtrType->is_pointer() && structPtrType->get_base_type()->is_struct()))
        SemanticError::raise(n->get_loc(), "bad indirect field ref of non-ptr-struct type\n");

    // search for this field
    std::shared_ptr<Type> structType = structPtrType->get_base_type();
    for (auto i = 0; i < structType->get_num_members(); i++) {
        Member member = structType->get_member(i);
        if (member.get_name() == fieldName) {
            std::shared_ptr<Type> type = member.get_type();
            // handle array as pointer
            if (type->is_array()) {
                std::shared_ptr<Type> ptrType(new PointerType(type->get_base_type()));
                n->setType(ptrType);
            } else
                n->setType(type);
//            std::printf("indirect field: type: %s\n", n->getType()->as_str().c_str());
            return;
        }
    }
    // wrong field name
    SemanticError::raise(n->get_loc(), "wrong ptr-struct field name\n");
}

void SemanticAnalysis::visit_array_element_ref_expression(Node *n) {
    // 2 kids
    // 0: array name
    // 1: index
//    std::printf("array: %s\n", n->get_kid(0)->get_kid(0)->get_str().c_str());
    visit(n->get_kid(0));
    visit(n->get_kid(1));

//    std::string arrayName = n->get_kid(0)->get_str();
//    std::printf("in array element, array name = %s\n", arrayName.c_str());
//    Symbol *arraySymbol = m_cur_symtab->lookup_recursive(arrayName);
//    if (arraySymbol == nullptr)
//        SemanticError::raise(n->get_loc(), "array does not exist\n");
    std::shared_ptr<Type> ptrType = n->get_kid(0)->getType();

    std::shared_ptr<Type> indexType = n->get_kid(1)->getType();
//    std::printf("array symbol: %s, index symbol: %s\n", arraySymbol->get_name().c_str(),
//                indexSymbol->get_name().c_str());
    if (!(indexType->is_integral() && ptrType->is_pointer()))
        SemanticError::raise(n->get_loc(), "wrong array element ref\n");
    n->setType(ptrType->get_base_type());
//    std::printf("in array element, n->type: %s\n", n->getType()->as_str().c_str());
//    n->set_str(arrayName);
    n->setSymbol(n->get_kid(0)->getSymbol());
}

void SemanticAnalysis::visit_variable_ref(Node *n) {
    // find reference in symbol table
    std::string name = n->get_kid(0)->get_str();
//    std::printf("in variable ref, name: %s\n", name.c_str());
    Symbol *symbol = m_cur_symtab->lookup_recursive(name);
    if (symbol == nullptr)
        SemanticError::raise(n->get_loc(), "variable reference not found\n");

    // if array appears, should be pointer to base_type
    if (symbol->get_type()->is_array()) {
        std::shared_ptr<Type> ptrType(new PointerType(symbol->get_type()->get_base_type()));
        n->setType(ptrType);
        n->setIsArray(true);
    }
    n->setSymbol(symbol);
//    std::printf("in variable ref, symbol: %s\n", symbol->get_name().c_str());

//     for array name
//    n->set_str(name);
}

void SemanticAnalysis::visit_literal_value(Node *n) {
//    std::printf("in visit literal value\n");
//    n->setIsLiteral(true);
    Node *kid = n->get_kid(0);
    int tag = kid->get_tag();
    switch (tag) {
        case TOK_STR_LIT: {
            //            n->setLiteralKind(LiteralValueKind::STRING);
            LiteralValue strValue = LiteralValue::from_str_literal(kid->get_str(), kid->get_loc());
            n->setLiteralValue(strValue);
            break;
        }
        case TOK_INT_LIT: {
            //            n->setLiteralKind(LiteralValueKind::INTEGER);
            LiteralValue intValue = LiteralValue::from_int_literal(kid->get_str(), kid->get_loc());
            n->setLiteralValue(intValue);
            break;
        }
        case TOK_CHAR_LIT: {
            //            n->setLiteralKind(LiteralValueKind::CHARACTER);
            LiteralValue charValue = LiteralValue::from_char_literal(kid->get_str(), kid->get_loc());
            n->setLiteralValue(charValue);
            break;
        }
        default:
            SemanticError::raise(n->get_loc(), "wrong type of literal value\n");
    }
}

void SemanticAnalysis::visit_return_expression_statement(Node *n) {
    Symbol *funcSymbol = n->getFuncSymbol();
    std::shared_ptr<Type> returnTypeFunc = funcSymbol->get_type()->get_base_type();
    visit(n->get_kid(0));
    // 2 cases:
    // return a literal value
    // return a variable ref
//    if (n->get_kid(0)->get_tag() == AST_LITERAL_VALUE) {
//        // return a literal value
//        visit_literal_value(n->get_kid(0));
//        LiteralValueKind literalKind = n->get_kid(0)->getLiteralKind();
//        if (literalKind == LiteralValueKind::STRING) {
//            // char*
//            if (!(returnTypeFunc->is_pointer() &&
//                  returnTypeFunc->get_base_type()->get_basic_type_kind() == BasicTypeKind::CHAR))
//                SemanticError::raise(n->get_loc(), "bad return string literal\n");
//        } else if (literalKind == LiteralValueKind::INTEGER) {
//            // int, short, long
//            if (!(returnTypeFunc->is_basic() && (returnTypeFunc->get_basic_type_kind() == BasicTypeKind::INT ||
//                                                 returnTypeFunc->get_basic_type_kind() == BasicTypeKind::LONG ||
//                                                 returnTypeFunc->get_basic_type_kind() == BasicTypeKind::SHORT)))
//                SemanticError::raise(n->get_loc(), "bad return integer literal\n");
//        } else if (literalKind == LiteralValueKind::CHARACTER) {
//            // char
//            if (!(returnTypeFunc->is_basic() && returnTypeFunc->get_basic_type_kind() == BasicTypeKind::CHAR))
//                SemanticError::raise(n->get_loc(), "bad return char literal\n");
//        }
//    } else {
    // return a variable ref
    std::shared_ptr<Type> returnTypeReal = n->get_kid(0)->getType();
//    std::printf("in return expression, func type = %s, return type = %s", returnTypeFunc->as_str().c_str(), returnTypeReal->as_str().c_str());
    if (!(returnTypeFunc->is_same(returnTypeReal.get())))
        SemanticError::raise(n->get_loc(), "wrong return type\n");
//    }
}

// TODO: implement helper functions
void SemanticAnalysis::basicDefAnalysis(Node *n, int *array) {
    // 0: void
    // 1: char
    // 2: int
    // 3: long
    // 4: short
    // 5: signed
    // 6: unsigned
    // 7: const
    // 8: volatile

    // rules:
    // only one of char, int, or void may be used
    // void cannot be combined with any other keywords
    // long and short can only be used with int or nothing
    // long and short are mutually exclusive
    // signed and unsigned are mutually exclusive
    // can have multiple pointers
    // const volatile are not mutually exclusive

    // only one of char, int, or void may be used
    if ((array[0] + array[1] + array[2]) > 1)
        SemanticError::raise(n->get_loc(), "only one of char, int, or void may be used\n");

    // signed and unsigned are mutually exclusive
    if ((array[5] + array[6]) > 1)
        SemanticError::raise(n->get_loc(), "signed and unsigned are mutually exclusive\n");

    // void cannot be combined with any other keywords
    if (array[0]) {
        int sum = 0;
        for (int i = 1; i < 9; i++)
            sum += array[i];
        if (sum)
            SemanticError::raise(n->get_loc(), "void cannot be combined with any other keywords\n");
    }

    // long and short are mutually exclusive
    if ((array[3] + array[4]) > 1)
        SemanticError::raise(n->get_loc(), "long and short are mutually exclusive\n");

    // long and short can only be used with int or nothing
    if ((array[3] + array[4]) == 1) {
        if (array[1])
            // char
            SemanticError::raise(n->get_loc(), "long and short can only be used with int or nothing\n");
    }

    if (array[0])
        visit_tokVoid(n);    // void
    else if (array[1])
        visit_tokChar(n);    // char
    else if (array[3])
        visit_tokLong(n);    // long
    else if (array[4])
        visit_tokShort(n);    // short
    else if (array[2] || (array[5] + array[6]))
        visit_tokInt(n);    // int

    // add unsigned
    if (array[6])
        n->getType()->setUnsigned();

    // add qualifier
    if ((array[7] + array[8]) > 0) {
        std::shared_ptr<QualifiedType> qualType(
                new QualifiedType(n->getType(), array[7] ? TypeQualifier::CONST : TypeQualifier::VOLATILE));
        n->setType(qualType);
    }
}

void SemanticAnalysis::visit_tokInt(Node *n) {
//    std::printf("in visit tokint\n");
    std::shared_ptr<BasicType> intType(new BasicType(BasicTypeKind::INT, true));
    n->setType(intType);
//    std::printf("%s\n", n->getType()->as_str().c_str());
}

void SemanticAnalysis::visit_tokShort(Node *n) {
    std::shared_ptr<BasicType> shortType(new BasicType(BasicTypeKind::SHORT, true));
    n->setType(shortType);
}

void SemanticAnalysis::visit_tokLong(Node *n) {
    std::shared_ptr<BasicType> longType(new BasicType(BasicTypeKind::LONG, true));
    n->setType(longType);
}

void SemanticAnalysis::visit_tokChar(Node *n) {
    std::shared_ptr<BasicType> charType(new BasicType(BasicTypeKind::CHAR, true));
    n->setType(charType);
}

void SemanticAnalysis::visit_tokVoid(Node *n) {
    std::shared_ptr<BasicType> voidType(new BasicType(BasicTypeKind::VOID, true));
    n->setType(voidType);
}

void SemanticAnalysis::visit_pointer_declarator(Node *n) {
    // modify type to pointer
    std::shared_ptr<PointerType> pointerType(new PointerType(n->getType()));
    n->setType(pointerType);
    n->get_kid(0)->setType(pointerType);

    passMember(n);
    visit(n->get_kid(0));
    n->setSymbol(n->get_kid(0)->getSymbol());
}

void SemanticAnalysis::visit_named_declarator(Node *n) {
    // add entry to symbol table
    std::string name = n->get_kid(0)->get_str();
    // make sure name is not used
    if (m_cur_symtab->has_symbol_local(name))
        SemanticError::raise(n->get_loc(), "symbol name already used\n");

    m_cur_symtab->define(SymbolKind::VARIABLE, name, n->getType());

    // add symbol to current node
    Symbol *symbol = m_cur_symtab->lookup_local(name);
    n->setSymbol(symbol);
//    std::printf("named decl, name: %s, type: %s\n", n->get_kid(0)->get_str().c_str(), n->getType()->as_str().c_str());

//    // set storage type of struct members as memory
//    if (n->getIsMember() && n->getMemberName().rfind("struct ", 0) == 0) {
//        symbol->setReqStorKind()
//    }

    callAddMember(n);
}

void SemanticAnalysis::visit_array_declarator(Node *n) {
    // modify type to array
    unsigned size = std::stoul(n->get_kid(1)->get_str());
    std::shared_ptr<ArrayType> arrayType(new ArrayType(n->getType(), size));
    n->setType(arrayType);
    n->get_kid(0)->setType(arrayType);

    passMember(n);
    visit(n->get_kid(0));
    n->setSymbol(n->get_kid(0)->getSymbol());
}

void SemanticAnalysis::visit_field_definition_list(Node *n) {
    // should call add_member()
//    n->preorder(*helperSetIsMember);
    passMember(n);

    for (auto i = 0; i < n->get_num_kids(); i++) {
        Node *kid = n->get_kid(i);
        visit(kid);
    }
}

void SemanticAnalysis::enterScope() {
    SymbolTable *nextScope = new SymbolTable(m_cur_symtab);
    m_cur_symtab = nextScope;
}

void SemanticAnalysis::leaveScope() {
    m_cur_symtab = m_cur_symtab->get_parent();
    assert(m_cur_symtab != nullptr);
}

void SemanticAnalysis::passMember(Node *n) {
    if (n->getIsMember()) {
        for (auto i = 0; i < n->get_num_kids(); i++) {
            Node *kid = n->get_kid(i);
            kid->setIsMember(true);
            kid->setMemberName(n->getMemberName());
        }
    }
}

void SemanticAnalysis::callAddMember(Node *n) {
    if (n->getIsMember()) {
        // get name from kid 0 literal
        Member member = Member(n->get_kid(0)->get_str(), n->getType());
        Symbol *symbolBelong = m_cur_symtab->lookup_recursive(n->getMemberName());
        if (!symbolBelong)
            RuntimeError::raise("symbolBelong doesn't exist\n");
//        std::printf("in callAddMember, symbolName = %s\n", n->getMemberName().c_str());
//        std::printf("callAddMember, member name: %s, member type: %s\n", member.get_name().c_str(), member.get_type()->as_str().c_str());
        symbolBelong->get_type()->add_member(member);
    }
}

void SemanticAnalysis::visit_Assign(Node *n) {
    Node *lnode = n->get_kid(1);
    Node *rnode = n->get_kid(2);
    isLvalue(lnode);
    std::shared_ptr<Type> ltype = lnode->getType();
    std::shared_ptr<Type> rtype = rnode->getType();
    analyzeAssignRef(ltype, rtype, n);
    // check implicit conversion
    // both integral, right convert to left
    if ((ltype->is_integral() && rtype->is_integral()) && !rtype->is_same(ltype.get())) {
        n->set_kid(2, implicit_conversion(rnode, ltype));
    }
}

void SemanticAnalysis::analyzeAssignRef(std::shared_ptr<Type> ltype, std::shared_ptr<Type> rtype, Node *n) {
    if (ltype->is_const())
        SemanticError::raise(n->get_loc(), "bad assign to const lvalue\n");
    else if ((ltype->is_array() && rtype->is_pointer()) &&
             (ltype->get_base_type()->is_same(rtype->get_base_type().get()))) {
        // good: array type in function parameters, pointer type in function call
    }
        // pointer with non-pointer is bad
    else if ((ltype->is_pointer() && !rtype->is_pointer()) || (!ltype->is_pointer() && rtype->is_pointer()))
        SemanticError::raise(n->get_loc(), "bad assign: pointer with non-pointer\n");
        // same struct type is good
    else if (ltype->is_struct()) {
        if (!ltype->is_same(rtype.get()))
            SemanticError::raise(n->get_loc(), "bad assign for struct\n");
    }
        // both short, int, long, char is good
    else if (ltype->is_basic() && ltype->get_basic_type_kind() != BasicTypeKind::VOID) {
        if (!(rtype->is_basic() && rtype->get_basic_type_kind() != BasicTypeKind::VOID))
            SemanticError::raise(n->get_loc(), "bad assign for basic types\n");
    }
        // both pointers
        // same basic type
        // ltype does not lack qualifiers
    else if (ltype->is_pointer() && rtype->is_pointer()) {
//        if (!ltype->is_same(rtype.get()))
//            SemanticError::raise(n->get_loc(), "bad assign for pointers\n");
        if (ltype->get_base_type()->get_unqualified_type()->is_same(rtype->get_base_type()->get_unqualified_type())) {
            if ((rtype->get_base_type()->is_const() && !ltype->get_base_type()->is_const()) ||
                (rtype->get_base_type()->is_volatile() && !ltype->get_base_type()->is_volatile()))
                SemanticError::raise(n->get_loc(), "bad assign for pointers: ltype lacks qualifiers\n");
        } else
            SemanticError::raise(n->get_loc(), "bad assign for pointers with different basic type\n");
    }
        // other cases
    else
        SemanticError::raise(n->get_loc(), "bad assign for other cases\n");
}

void SemanticAnalysis::isLvalue(Node *n) {
    int tag = n->get_tag();
    // cannot assign to an array
//    std::printf("in isLvalue, n->hasSymbol: %d\n", n->hasSymbol());
    if (n->getIsArray())
        SemanticError::raise(n->get_loc(), "assign to non-lvavlue (array)\n");
    if (tag == AST_VARIABLE_REF || tag == AST_FIELD_REF_EXPRESSION || tag == AST_INDIRECT_FIELD_REF_EXPRESSION ||
        tag == AST_ARRAY_ELEMENT_REF_EXPRESSION) {

    } else if (tag == AST_UNARY_EXPRESSION && n->get_kid(0)->get_tag() == TOK_ASTERISK) {
        // pointer deref
    } else
        SemanticError::raise(n->get_loc(), "assign to non-lvavlue\n");
}

void SemanticAnalysis::passFuncSymbAllNodes(Node *n) {
    Symbol *funcSymbol = n->getFuncSymbol();
    for (auto i = 0; i < n->get_num_kids(); i++) {
        Node *kid = n->get_kid(i);
        kid->setFuncSymbol(funcSymbol);
        passFuncSymbAllNodes(kid);
    }
}

//Node *SemanticAnalysis::promote_to_int(Node *n) {
//    assert(n->get_type()->is_integral());
//    assert(n->get_type()->get_basic_type_kind() < BasicTypeKind::INT);
//    std::shared_ptr<Type> type(new BasicType(BasicTypeKind::INT, n->get_type()->is_signed()));
//    return implicit_conversion(n, type);
//}

Node *SemanticAnalysis::implicit_conversion(Node *n, const std::shared_ptr<Type> &type) {
    std::unique_ptr<Node> conversion(new Node(AST_IMPLICIT_CONVERSION, {n}));
    conversion->setType(type);
    return conversion.release();
}

