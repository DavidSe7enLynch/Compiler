#include <cassert>
#include <cstdio>
#include "symtab.h"
#include "exceptions.h"

////////////////////////////////////////////////////////////////////////
// Symbol implementation
////////////////////////////////////////////////////////////////////////

Symbol::Symbol(SymbolKind kind, const std::string &name, const std::shared_ptr <Type> &type, SymbolTable *symtab,
               bool is_defined)
        : m_kind(kind), m_name(name), m_type(type), m_symtab(symtab), m_is_defined(is_defined),
          m_storage(StorageKind::VREGISTER, -1) {
    // init of storage, set vreg to -1, which is impossible, waiting for future setStorage
//    m_storage = Storage(StorageKind::VREGISTER, -1);
}

Symbol::~Symbol() {
}

void Symbol::set_is_defined(bool is_defined) {
    m_is_defined = is_defined;
}

SymbolKind Symbol::get_kind() const {
    return m_kind;
}

const std::string &Symbol::get_name() const {
    return m_name;
}

std::shared_ptr <Type> Symbol::get_type() const {
    return m_type;
}

void Symbol::setType(std::shared_ptr <Type> type) {
    m_type = type;
}

SymbolTable *Symbol::get_symtab() const {
    return m_symtab;
}

bool Symbol::is_defined() const {
    return m_is_defined;
}

void Symbol::setStorage(StorageKind kind, int vreg) {
    m_storage = Storage(kind, vreg);
}

void Symbol::setStorage(StorageKind kind, unsigned int memOffset) {
//    std::printf("setStorage, memoffset: %d\n", memOffset);
    m_storage = Storage(kind, memOffset);
}

void Symbol::setStorage(StorageKind kind, std::string globalName) {
    m_storage = Storage(kind, globalName);
}

const Storage &Symbol::getStorage() const {
    return m_storage;
}

bool Symbol::isGlobal() const {
    assert(m_symtab != nullptr);
    return m_symtab->isGlobal();
}

void Symbol::setReqStorKind(StorageKind kind) {
    m_reqStorKind = kind;
}

StorageKind Symbol::getReqStorKind() {
    return m_reqStorKind;
}

////////////////////////////////////////////////////////////////////////
// SymbolTable implementation
////////////////////////////////////////////////////////////////////////

SymbolTable::SymbolTable(SymbolTable *parent)
        : m_parent(parent), m_has_params(false) {
}

SymbolTable::~SymbolTable() {
    for (auto i = m_symbols.begin(); i != m_symbols.end(); ++i) {
        delete *i;
    }
}

SymbolTable *SymbolTable::get_parent() const {
    return m_parent;
}

bool SymbolTable::has_params() const {
    return m_has_params;
}

void SymbolTable::set_has_params(bool has_params) {
    m_has_params = has_params;
}

bool SymbolTable::has_symbol_local(const std::string &name) const {
    return lookup_local(name) != nullptr;
}

Symbol *SymbolTable::lookup_local(const std::string &name) const {
    auto i = m_lookup.find(name);
    return (i != m_lookup.end()) ? m_symbols[i->second] : nullptr;
}

Symbol *SymbolTable::declare(SymbolKind sym_kind, const std::string &name, const std::shared_ptr <Type> &type) {
    Symbol *sym = new Symbol(sym_kind, name, type, this, false);
    add_symbol(sym);
    return sym;
}

Symbol *SymbolTable::define(SymbolKind sym_kind, const std::string &name, const std::shared_ptr <Type> &type) {
    Symbol *sym = new Symbol(sym_kind, name, type, this, true);
    add_symbol(sym);
    return sym;
}

Symbol *SymbolTable::lookup_recursive(const std::string &name) const {
    const SymbolTable *scope = this;

    while (scope != nullptr) {
        Symbol *sym = scope->lookup_local(name);
        if (sym != nullptr)
            return sym;
        scope = scope->get_parent();
    }

    return nullptr;
}

void SymbolTable::set_fn_type(const std::shared_ptr <Type> &fn_type) {
    assert(!m_fn_type);
    m_fn_type = fn_type;
}

const Type *SymbolTable::get_fn_type() const {
    const SymbolTable *symtab = this;
    while (symtab != nullptr) {
        if (m_fn_type)
            return m_fn_type.get();
        symtab = symtab->m_parent;
    }

    return nullptr;
}

void SymbolTable::add_symbol(Symbol *sym) {
    assert(!has_symbol_local(sym->get_name()));

    unsigned pos = unsigned(m_symbols.size());
    m_symbols.push_back(sym);
    m_lookup[sym->get_name()] = pos;

//    // Assignment 3 only: print out symbol table entries as they are added
//    printf("%d|", get_depth());
//    printf("%s|", sym->get_name().c_str());
//    switch (sym->get_kind()) {
//        case SymbolKind::FUNCTION:
//            printf("function|");
//            break;
//        case SymbolKind::VARIABLE:
//            printf("variable|");
//            break;
//        case SymbolKind::TYPE:
//            printf("type|");
//            break;
//        default:
//            assert(false);
//    }
//    printf("%s\n", sym->get_type()->as_str().c_str());
}

int SymbolTable::get_depth() const {
    int depth = 0;

    const SymbolTable *symtab = m_parent;
    while (symtab != nullptr) {
        ++depth;
        symtab = symtab->m_parent;
    }

    return depth;
}

bool SymbolTable::isGlobal() const {
    return (m_parent == nullptr);
}


////////////////////////////////////////////////////////////////////////
// Storage implementation
////////////////////////////////////////////////////////////////////////


Storage::Storage(StorageKind kind, int vreg) : m_kind(kind), m_vreg(vreg), m_memOffset(0), m_globalName("") {
    assert(m_kind == StorageKind::VREGISTER);
}

Storage::Storage(StorageKind kind, unsigned int memOffset) : m_kind(kind), m_vreg(0), m_memOffset(memOffset),
                                                             m_globalName("") {
    assert(m_kind == StorageKind::MEMORY);
}

Storage::Storage(StorageKind kind, std::string globalName) : m_kind(kind), m_vreg(0), m_memOffset(0),
                                                             m_globalName(globalName) {
    assert(m_kind == StorageKind::GLOBAL);
}

StorageKind Storage::getKind() const {
    switch (m_kind) {
        case StorageKind::VREGISTER:
            assert(m_memOffset == 0);
            assert(m_globalName == "");
            return m_kind;
        case StorageKind::MEMORY:
            assert(m_vreg == 0);
            assert(m_globalName == "");
            return m_kind;
        case StorageKind::GLOBAL:
            assert(m_vreg == 0);
            assert(m_memOffset == 0);
            return m_kind;
        default:
            RuntimeError::raise("wrong storage type\n");
    }
}

int Storage::getVreg() const {
    assert(m_memOffset == 0);
    assert(m_globalName == "");
    assert(m_kind == StorageKind::VREGISTER);
    return m_vreg;
}

unsigned Storage::getMemOffset() const {
    assert(m_vreg == 0);
    assert(m_globalName == "");
    assert(m_kind == StorageKind::MEMORY);
    return m_memOffset;
}

std::string Storage::getGlobalName() const {
    assert(m_vreg == 0);
    assert(m_memOffset == 0);
    assert(m_kind == StorageKind::GLOBAL);
    return m_globalName;
}

