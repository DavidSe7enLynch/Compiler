#include <cassert>
#include "node.h"
#include "instruction.h"
#include "highlevel.h"
#include "ast.h"
#include "parse.tab.h"
#include "grammar_symbols.h"
#include "exceptions.h"
#include "highlevel_codegen.h"

namespace {

// Adjust an opcode for a basic type
    HighLevelOpcode get_opcode(HighLevelOpcode base_opcode, const std::shared_ptr <Type> &type) {
        if (type->is_basic())
            return static_cast<HighLevelOpcode>(int(base_opcode) + int(type->get_basic_type_kind()));
        else if (type->is_pointer())
            return static_cast<HighLevelOpcode>(int(base_opcode) + int(BasicTypeKind::LONG));
        else
            RuntimeError::raise("attempt to use type '%s' as data in opcode selection", type->as_str().c_str());
    }

}

HighLevelCodegen::HighLevelCodegen(int next_label_num)
        : m_next_label_num(next_label_num), m_hl_iseq(new InstructionSequence()), m_nextOrigVreg(0), m_nextCurVreg(0) {
}

HighLevelCodegen::~HighLevelCodegen() {
}

void HighLevelCodegen::visit_function_definition(Node *n) {
    // 4 kids
    // 1: return type
    // 2: name: TOK_IDENT
    // 3: parameter list
    // 4: statement list

    // generate the name of the label that return instructions should target
    std::string fn_name = n->get_kid(1)->get_str();
    m_return_label_name = ".L" + fn_name + "_return";

    // need to calculate total local storage of the function
    unsigned total_local_storage = n->getLocalTotalStorage();
    // need to get nextVreg
    m_nextCurVreg = n->get_kid(3)->getNextVreg();

    // enter
    m_hl_iseq->append(new Instruction(HINS_enter, Operand(Operand::IMM_IVAL, total_local_storage)));

    // store parameters in vregs
    Node *paramList = n->get_kid(2);
    for (auto i = 0; i < paramList->get_num_kids(); i++) {
        Node *param = paramList->get_kid(i);
        Symbol *symbol = param->getSymbol();

        HighLevelOpcode movOpcode = get_opcode(HINS_mov_b, symbol->get_type());

        Operand newOperand = nextTempOperand();
        Operand origOperand = Operand(Operand::VREG, symbol->getStorage().getVreg());

        m_hl_iseq->append(new Instruction(movOpcode, newOperand, origOperand));

        symbol->setStorage(StorageKind::VREGISTER, newOperand.get_base_reg());
//        std::printf("dealing with param No. %d, store in vreg %d\n", i, symbol->getStorage().getVreg());
    }

    // now we have store all local variables, origVreg starts here
    m_nextOrigVreg = m_nextCurVreg;

    // visit body
    visit(n->get_kid(3));

    // leave
    m_hl_iseq->define_label(m_return_label_name);
    m_hl_iseq->append(new Instruction(HINS_leave, Operand(Operand::IMM_IVAL, total_local_storage)));
    m_hl_iseq->append(new Instruction(HINS_ret));
}

void HighLevelCodegen::visit_expression_statement(Node *n) {
    visit_children(n);
    m_nextCurVreg = m_nextOrigVreg;
}

void HighLevelCodegen::visit_return_statement(Node *n) {
    // jump to the return label
    m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, m_return_label_name)));
}

void HighLevelCodegen::visit_return_expression_statement(Node *n) {
    // A possible implementation:

    Node *expr = n->get_kid(0);

    // generate code to evaluate the expression
    visit(expr);

    // move the computed value to the return value vreg
    HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, expr->getType());
    m_hl_iseq->append(new Instruction(mov_opcode, Operand(Operand::VREG, LocalStorageAllocation::VREG_RETVAL),
                                      expr->getOperand()));

    // jump to the return label
    visit_return_statement(n);

}

void HighLevelCodegen::visit_while_statement(Node *n) {
    // jmp to label 1
    // label 0: statements
    // label 1: while expression
    // jmp to label 0
    std::string stmtLabel = next_label();
    std::string exprLabel = next_label();

    // jmp to label 1
    m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, exprLabel)));

    // label 0: statements
    m_hl_iseq->define_label(stmtLabel);
    visit(n->get_kid(1));

    // label 1: while expression
    m_hl_iseq->define_label(exprLabel);
    visit(n->get_kid(0));

    // jmp to label 0
    m_hl_iseq->append(new Instruction(HINS_cjmp_t, n->get_kid(0)->getOperand(), Operand(Operand::LABEL, stmtLabel)));
}

void HighLevelCodegen::visit_do_while_statement(Node *n) {
    // label 0: statements
    // while expression
    // jmp to label 0
    std::string stmtLabel = next_label();

    // label 0: statements
    m_hl_iseq->define_label(stmtLabel);
    visit(n->get_kid(0));

    // while expression
    visit(n->get_kid(1));

    // jmp to label 0
    m_hl_iseq->append(new Instruction(HINS_cjmp_t, n->get_kid(1)->getOperand(), Operand(Operand::LABEL, stmtLabel)));
}

void HighLevelCodegen::visit_for_statement(Node *n) {
    // 4 kids
    // 0-2: 3 expressions
    // 3: statement list

    // for expression (kid 0)
    // jmp to label 1
    // label 0: statements (kid 3)
    // for expression (kid 2)
    // label 1: for expression (kid 1)
    // jmp to label 0
    std::string stmtLabel = next_label();
    std::string exprLabel = next_label();

    // for expression (kid 0)
    visit(n->get_kid(0));

    // jmp to label 1
    m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, exprLabel)));

    // label 0: statements (kid 3)
    m_hl_iseq->define_label(stmtLabel);
    visit(n->get_kid(3));

    // for expression (kid 2)
    visit(n->get_kid(2));

    // label 1: for expression (kid 1)
    m_hl_iseq->define_label(exprLabel);
    visit(n->get_kid(1));

    // jmp to label 0
    m_hl_iseq->append(new Instruction(HINS_cjmp_t, n->get_kid(1)->getOperand(), Operand(Operand::LABEL, stmtLabel)));
}

void HighLevelCodegen::visit_if_statement(Node *n) {
    // 2 kids
    // 0: if expression
    // 1: statement list

    // if expression
    // if false, jump to label 0
    // statement list
    // label 0
    std::string afterLabel = next_label();

    // if expression
    visit(n->get_kid(0));

    // if false, jump to label 0
    m_hl_iseq->append(new Instruction(HINS_cjmp_f, n->get_kid(0)->getOperand(), Operand(Operand::LABEL, afterLabel)));

    // statement list
    visit(n->get_kid(1));

    // label 0
    m_hl_iseq->define_label(afterLabel);
}

void HighLevelCodegen::visit_if_else_statement(Node *n) {
    // 3 kids
    // 0: if expression
    // 1: statement list if
    // 2: statement list else

    // if expression
    // if false, jump to label 0
    // statement list if (kid 1)
    // jmp to label 1
    // label 0: statement list else (kid 2)
    // label 1
    std::string elseLabel = next_label();
    std::string afterLabel = next_label();

    // if expression
    visit(n->get_kid(0));

    // if false, jump to label 0
    m_hl_iseq->append(new Instruction(HINS_cjmp_f, n->get_kid(0)->getOperand(), Operand(Operand::LABEL, elseLabel)));

    // statement list if (kid 1)
    visit(n->get_kid(1));

    // jmp to label 1
    m_hl_iseq->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, afterLabel)));

    // label 0: statement list else (kid 2)
    m_hl_iseq->define_label(elseLabel);
    visit(n->get_kid(2));

    // label 1
    m_hl_iseq->define_label(afterLabel);
}

void HighLevelCodegen::visit_binary_expression(Node *n) {
    int tag = n->get_kid(0)->get_tag();
    Node *lnode = n->get_kid(1);
    Node *rnode = n->get_kid(2);
    visit(lnode);
    visit(rnode);
    switch (tag) {
        case TOK_ASSIGN: {
            HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, lnode->getType());
            m_hl_iseq->append(new Instruction(mov_opcode, lnode->getOperand(), rnode->getOperand()));
            break;
        }
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_ASTERISK:
        case TOK_DIVIDE:
        case TOK_MOD:
        case TOK_LT:
        case TOK_LTE:
        case TOK_GT:
        case TOK_GTE:
        case TOK_EQUALITY:
        case TOK_INEQUALITY: {
            Operand dest = nextTempOperand();
            HighLevelOpcode opcode;
            switch (tag) {
                case TOK_PLUS: opcode = get_opcode(HINS_add_b, lnode->getType()); break;
                case TOK_MINUS: opcode = get_opcode(HINS_sub_b, lnode->getType()); break;
                case TOK_ASTERISK: opcode = get_opcode(HINS_mul_b, lnode->getType()); break;
                case TOK_DIVIDE: opcode = get_opcode(HINS_div_b, lnode->getType()); break;
                case TOK_MOD: opcode = get_opcode(HINS_mod_b, lnode->getType()); break;
                case TOK_LT: opcode = get_opcode(HINS_cmplt_b, lnode->getType()); break;
                case TOK_LTE: opcode = get_opcode(HINS_cmplte_b, lnode->getType()); break;
                case TOK_GT: opcode = get_opcode(HINS_cmpgt_b, lnode->getType()); break;
                case TOK_GTE: opcode = get_opcode(HINS_cmpgte_b, lnode->getType()); break;
                case TOK_EQUALITY: opcode = get_opcode(HINS_cmpeq_b, lnode->getType()); break;
                case TOK_INEQUALITY: opcode = get_opcode(HINS_cmpneq_b, lnode->getType()); break;
                default:
                    SemanticError::raise(n->get_loc(), "wrong type of binary operator\n");
            }
            m_hl_iseq->append(new Instruction(opcode, dest, lnode->getOperand(), rnode->getOperand()));
            n->setOperand(dest);
            break;
        }

        default:
            SemanticError::raise(n->get_loc(), "wrong type of binary operator\n");
    }
}

void HighLevelCodegen::visit_function_call_expression(Node *n) {
    // 2 kids
    // 0: variable ref: func name
    // 1: argument list

    // move arguments to argument vregs
    // call function name
    // assign operand vr0, which is the return value of the function, to this node

    // move arguments to argument vregs
//    std::printf("in func call\n");
    Node *argList = n->get_kid(1);
    for (auto i = 0; i < argList->get_num_kids(); i++) {
        Node *arg = argList->get_kid(i);
        visit(arg);
        HighLevelOpcode opcode = get_opcode(HINS_mov_b, arg->getType());
        m_hl_iseq->append(new Instruction(opcode, Operand(Operand::VREG, i + 1), arg->getOperand()));
    }

    // call function name
    std::string name = n->get_kid(0)->get_kid(0)->get_str();
    m_hl_iseq->append(new Instruction(HINS_call, Operand(Operand::LABEL, name)));

    // assign operand vr0, which is the return value of the function, to this node
    n->setOperand(Operand(Operand::VREG, 0));
}

void HighLevelCodegen::visit_array_element_ref_expression(Node *n) {
    // 2 kids
    // 0: var ref: array name
    // 1: var ref / literal val: index

    Node *arr = n->get_kid(0);
    Node *idx = n->get_kid(1);

    // get addr of first element
    // calculate addr of this element

    visit(arr);
    visit(idx);

    // if it is a struct field
    // if 2-D array
    // then kid0 type must be array
    // set operand of kid0 back to vreg (from memref)
    if (arr->getType()->is_array()) {
        arr->setOperand(arr->getOperand().memref_to());
    }

    // n has type of base_type, which contains the info of size
    int sizeElem = n->getType()->get_storage_size();

    Operand tempOperand1 = nextTempOperand();
    Operand tempOperand2 = nextTempOperand();

    // calculate address, must use 64bit

//    HighLevelOpcode mulOpcode = get_opcode(HINS_mul_b, n->getType());
    HighLevelOpcode mulOpcode = HINS_mul_q;
    m_hl_iseq->append(new Instruction(mulOpcode,
                                      tempOperand1,
                                      idx->getOperand(),
                                      Operand(Operand::IMM_IVAL, sizeElem)));

//    HighLevelOpcode addOpcode = get_opcode(HINS_add_b, n->getType());
    HighLevelOpcode addOpcode = HINS_add_q;
    m_hl_iseq->append(new Instruction(addOpcode,
                                      tempOperand2,
                                      tempOperand1,
                                      arr->getOperand()));

//    std::printf("array ele\n");
    n->setOperand(tempOperand2.to_memref());
//    n->setOperand(setToMemref(tempOperand2, n));
}

void HighLevelCodegen::visit_variable_ref(Node *n) {
    // get storage, set operand
    Symbol *symbol = n->getSymbol();
    const Storage &storage = symbol->getStorage();
    StorageKind kind = storage.getKind();
    switch (kind) {
        case StorageKind::GLOBAL:
            n->setOperand(Operand()); // ?
            break;
        case StorageKind::VREGISTER:
            n->setOperand(Operand(Operand::VREG, storage.getVreg()));
            break;
        case StorageKind::MEMORY: {
            // store memory offset into a vreg
//            std::printf("var ref memory: %s, offset: %d\n", symbol->get_name().c_str(), storage.getMemOffset());
            Operand tempOperand = nextTempOperand();
            m_hl_iseq->append(new Instruction(HINS_localaddr,
                                              tempOperand,
                                              Operand(Operand::IMM_IVAL, storage.getMemOffset())));
            // if it is array, then keep it as pointer to first element, i.e. the address of first element
            if (symbol->get_type()->is_array()) {
                n->setOperand(tempOperand);
            } else {
//                std::printf("var ref, Operand: %d\n", tempOperand.get_kind() == Operand::VREG);
                n->setOperand(tempOperand.to_memref());
//                n->setOperand(setToMemref(tempOperand, n));
            }
            break;
        }
        default:
            RuntimeError::raise("wrong storage kind\n");
    }
}

void HighLevelCodegen::visit_literal_value(Node *n) {
    // A partial implementation (note that this won't work correctly
    // for string constants!):
    LiteralValue val = n->getLiteralValue();
    Operand dest = nextTempOperand();
    HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->getType());
//    std::printf("highcodegen, litval, type: %s\n", n->getType()->as_str().c_str());
    if (val.get_kind() == LiteralValueKind::INTEGER) {
        m_hl_iseq->append(new Instruction(mov_opcode, dest, Operand(Operand::IMM_IVAL, val.get_int_value())));
    } else if (val.get_kind() == LiteralValueKind::STRING) {
        m_hl_iseq->append(new Instruction(mov_opcode, dest, n->getOperand()));
    } else if (val.get_kind() == LiteralValueKind::CHARACTER) {
        m_hl_iseq->append(new Instruction(mov_opcode, dest, Operand(Operand::IMM_IVAL, val.get_char_value())));
    }
    n->setOperand(dest);
}

void HighLevelCodegen::visit_unary_expression(Node *n) {
    // ampersand &
    // asterisk *
    // minus -
    Node *op = n->get_kid(0);
    Node *val = n->get_kid(1);
    visit(val);
    int tag = op->get_tag();
    switch (tag) {
        case TOK_AMPERSAND:
            n->setOperand(val->getOperand().memref_to());
            break;
        case TOK_ASTERISK: {
            if (!val->getOperand().is_memref()) {
                // 1-layer ptr
                n->setOperand(val->getOperand().to_memref());
            } else {
                // multi-layer ptr
//                std::printf("**pp, base type = %s, type = %s\n", val->getType()->get_base_type()->as_str().c_str(), val->getType()->as_str().c_str());
                // do not need base type, because it is multi-layer ptr, type has already been reduced
                HighLevelOpcode movOpcode = get_opcode(HINS_mov_b, val->getType());
                Operand tempOperand = nextTempOperand();
                m_hl_iseq->append(new Instruction(movOpcode, tempOperand, val->getOperand()));
                n->setOperand(tempOperand.to_memref());
            }
            break;
        }
        case TOK_MINUS: {
            // neg
            HighLevelOpcode negOpcode = get_opcode(HINS_neg_b, val->getType());
            Operand tempOperand = nextTempOperand();
            m_hl_iseq->append(new Instruction(negOpcode, tempOperand, val->getOperand()));
            n->setOperand(tempOperand);
            break;
        }
        default:
            SemanticError::raise(n->get_loc(), "wrong unary operator\n");
    }
}

void HighLevelCodegen::visit_field_ref_expression(Node *n) {
    // 2 kids
    // 0: var ref: struct name
    // 1: tok_ident: field name

    // load addr of struct
    // get relative mem offset of field

    // load addr of struct
    Node *structNode = n->get_kid(0);
    visit(structNode);

    std::shared_ptr<Type> structType = structNode->getType();

    // get relative mem offset of field
    std::string fieldName = n->get_kid(1)->get_str();
    const Member *member = structType->find_member(fieldName);
    unsigned fieldOffset = member->getOffset();
//    std::printf("member: %s, get offset, %d\n", fieldName.c_str(), fieldOffset);
    Operand tempOperand1 = nextTempOperand();
    Operand tempOperand2 = nextTempOperand();
    m_hl_iseq->append(new Instruction(HINS_mov_q, tempOperand1, Operand(Operand::IMM_IVAL, fieldOffset)));
    m_hl_iseq->append(new Instruction(HINS_add_q, tempOperand2, structNode->getOperand().memref_to(), tempOperand1));
    if (!member->get_type()->is_array())
        n->setOperand(tempOperand2.to_memref());
    else {
//        std::printf("member is array");
        n->setOperand(tempOperand2);
    }
//    n->setOperand(setToMemref(tempOperand2, n));
}

void HighLevelCodegen::visit_indirect_field_ref_expression(Node *n) {
    // 2 kids
    // 0: var ref: struct name
    // 1: tok_ident: field name

    // load addr of struct
    // get relative mem offset of field

    // load addr of struct
    Node *structNode = n->get_kid(0);
    visit(structNode);

    // pointer to struct -> struct
    std::shared_ptr<Type> structType = structNode->getType()->get_base_type();

    // get relative mem offset of field
    std::string fieldName = n->get_kid(1)->get_str();
    const Member *member = structType->find_member(fieldName);
    unsigned fieldOffset = member->getOffset();
//    std::printf("get offset, %d\n", fieldOffset);
    Operand tempOperand1 = nextTempOperand();
    Operand tempOperand2 = nextTempOperand();
    m_hl_iseq->append(new Instruction(HINS_mov_q, tempOperand1, Operand(Operand::IMM_IVAL, fieldOffset)));
    // different from direct field ref: don't need memref_to
    m_hl_iseq->append(new Instruction(HINS_add_q, tempOperand2, structNode->getOperand(), tempOperand1));
    if (!member->get_type()->is_array())
        n->setOperand(tempOperand2.to_memref());
    else
        n->setOperand(tempOperand2);
//    n->setOperand(setToMemref(tempOperand2, n));
}

void HighLevelCodegen::visit_implicit_conversion(Node *n) {
    // 1 kid
    // var ref

    // char: 1 byte: 8 bit: _b
    // short: 2 byte: 16 bit: _w
    // int: 4 byte: 32 bit: _l
    // long: 8 byte: 64 bit: _q

    //  HINS_sconv_bw,
    //  HINS_sconv_bl,
    //  HINS_sconv_bq,
    //  HINS_sconv_wl,
    //  HINS_sconv_wq,
    //  HINS_sconv_lq,
    //  HINS_uconv_bw,
    //  HINS_uconv_bl,
    //  HINS_uconv_bq,
    //  HINS_uconv_wl,
    //  HINS_uconv_wq,
    //  HINS_uconv_lq,

//    struct implicitConv {
//        bool isSigned;
//        BasicTypeKind origKind;
//        BasicTypeKind newKind;
//    };
//
//    struct implicitConv
//
//    std::map<implicitConv, HighLevelOpcode> mapImplicitConv {
//            {}
//    };

    visit(n->get_kid(0));
    std::shared_ptr<Type> origType = n->get_kid(0)->getType();
    std::shared_ptr<Type> newType = n->getType();
    bool isSigned = newType->is_signed();

    int base, origOffset, newOffset;

    newOffset = int(newType->get_basic_type_kind()) - int(origType->get_basic_type_kind()) - 1;
    // same type, no need to convert (signed -> unsigned)
    if (newOffset < 0) {
        n->setOperand(n->get_kid(0)->getOperand());
        return;
    }

    base = int(isSigned ? HINS_sconv_bw : HINS_uconv_bw);

    if (origType->get_basic_type_kind() == BasicTypeKind::CHAR) origOffset = 0;
    else if (origType->get_basic_type_kind() == BasicTypeKind::SHORT) origOffset = 3;
    else if (origType->get_basic_type_kind() == BasicTypeKind::INT) origOffset = 5;
    else SemanticError::raise(n->get_loc(), "wrong original type in implicit conversion\n");

    int opcodeInt = base + origOffset + newOffset;
    HighLevelOpcode opcode = static_cast<HighLevelOpcode>(opcodeInt);

    Operand dest = nextTempOperand();
    m_hl_iseq->append(new Instruction(opcode, dest, n->get_kid(0)->getOperand()));
    n->setOperand(dest);
}

std::string HighLevelCodegen::next_label() {
    std::string label = ".L" + std::to_string(m_next_label_num++);
    return label;
}

// TODO: additional private member functions

Operand HighLevelCodegen::nextTempOperand() {
    return Operand(Operand::VREG, m_nextCurVreg++);
}

Operand HighLevelCodegen::setToMemref(Operand operand, Node *n) {
    Operand tempOperand = nextTempOperand();
    HighLevelOpcode movOpcode = get_opcode(HINS_mov_b, n->getType());
    m_hl_iseq->append(new Instruction(movOpcode, tempOperand, operand.to_memref()));
    return tempOperand;
}
