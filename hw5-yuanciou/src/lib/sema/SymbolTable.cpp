#include "sema/SymbolTable.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <utility>

// ===========================================
// > Attribute
// ===========================================
const Constant *Attribute::constant() const {
    if (m_type != Tag::kConstantValue) {
        assert(false && "Try to extract constant from an attribute object that "
                        "is not for constant");
        return nullptr;
    }
    return m_constant_value_ptr;
}

const FunctionNode::DeclNodes *Attribute::parameters() const {
    if (m_type != Tag::kParameterDeclNodes) {
        assert(false && "Try to extract parameters from an attribute object"
                        "that is not for parameters");
        return nullptr;
    }
    return m_parameters_ptr;
}

// ===========================================
// > SymbolTable
// ===========================================
SymbolEntry *SymbolTable::addSymbol(const std::string &p_name,
                                    const SymbolEntry::KindEnum p_kind,
                                    const size_t p_level,
                                    const PType *const p_p_type,
                                    const Constant *const p_constant) {
    m_entries.emplace_back(
        new SymbolEntry(p_name, p_kind, p_level, p_p_type, p_constant));
    return m_entries.back().get();
}

SymbolEntry *
SymbolTable::addSymbol(const std::string &p_name,
                       const SymbolEntry::KindEnum p_kind,
                       const size_t p_level,
                       const PType *const p_p_type,
                       const FunctionNode::DeclNodes *const p_parameters) {
    m_entries.emplace_back(
        new SymbolEntry(p_name, p_kind, p_level, p_p_type, p_parameters));
    return m_entries.back().get();
}

SymbolEntry *SymbolTable::lookup(const std::string &p_name) const {
    for (const auto &entry : m_entries) {
        if (entry->getName() == p_name) {
            return entry.get();
        }
    }
    return nullptr;
}

void SymbolTable::dump() const {
    std::printf("=========================================================="
                "====================================================\n");
    std::printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
                "Attribute");
    std::printf("----------------------------------------------------------"
                "----------------------------------------------------\n");

    std::string type_string;
    auto construct_attr_string = [&type_string](const auto &p_entry_ptr) {
        if (p_entry_ptr->getKind() == SymbolEntry::KindEnum::kFunctionKind) {
            const FunctionNode::DeclNodes *const parameters_ptr =
                p_entry_ptr->getAttribute().parameters();
            type_string =
                FunctionNode::getParametersTypeString(*parameters_ptr);
            return type_string.c_str();
        } else {
            const Constant *const constant =
                p_entry_ptr->getAttribute().constant();
            if (constant) {
                return constant->getConstantValueCString();
            } else {
                return "";
            }
        }
    };

    auto dump_entry = [&construct_attr_string](const auto &p_entry_ptr) {
        static const char *kKindStrings[] = {"program",  "function", "parameter",
                                             "variable", "loop_var", "constant"};

        std::printf("%-33s", p_entry_ptr->getNameCString());
        std::printf("%-11s",
                    kKindStrings[static_cast<size_t>(p_entry_ptr->getKind())]);
        std::printf("%lu%-10s", p_entry_ptr->getLevel(),
                    (p_entry_ptr->getLevel() != 0) ? "(local)" : "(global)");
        std::printf("%-17s", p_entry_ptr->getTypePtr()->getPTypeCString());
        std::printf("%-11s\n", construct_attr_string(p_entry_ptr));
    };

    for_each(m_entries.begin(), m_entries.end(), dump_entry);

    std::printf("----------------------------------------------------------"
                "----------------------------------------------------\n");
}

// ===========================================
// > SymbolManager
// ===========================================
void SymbolManager::pushScope() {
    m_tables.emplace_back(std::make_unique<SymbolTable>());
}

void SymbolManager::pushScope(SymbolManager::Table p_table) {
    m_tables.push_back(std::move(p_table));
}

SymbolManager::Table SymbolManager::popScope() {
    assert(getCurrentTable() && "Shouldn't popScope() without pushing any scope");

    if (m_opt_dmp) {
        getCurrentTable()->dump();
    }

    auto table = std::move(m_tables.back());
    m_tables.pop_back();
    return table;
}

template <typename AttributeType>
SymbolEntry *SymbolManager::addSymbol(const std::string &p_name,
                                      const SymbolEntry::KindEnum p_kind,
                                      const PType *const p_p_type,
                                      const AttributeType *const p_attribute) {
    if (getCurrentTable()->lookup(p_name)) {
        return nullptr;
    }

    auto& current_table = m_tables.back();
    auto *entry = current_table->addSymbol(
        p_name, p_kind, getCurrentLevel(), p_p_type, p_attribute);
    return entry;
}

// explicit instantiation
template SymbolEntry *SymbolManager::addSymbol<Constant>(
    const std::string &, const SymbolEntry::KindEnum, const PType *const,
    const Constant *const);
template SymbolEntry *SymbolManager::addSymbol<FunctionNode::DeclNodes>(
    const std::string &, const SymbolEntry::KindEnum, const PType *const,
    const FunctionNode::DeclNodes *const);

SymbolEntry *SymbolManager::lookup(const std::string &p_name) const {
    for (auto it = m_tables.rbegin(); it != m_tables.rend(); ++it) {
        if (auto *entry = (*it)->lookup(p_name)) {
            return entry;
        }
    }
    return nullptr;
}

const SymbolTable *SymbolManager::getCurrentTable() const {
    if (m_tables.empty()) {
        return nullptr;
    }
    return m_tables.back().get();
}

size_t SymbolManager::getCurrentLevel() const {
    return m_tables.size() - 1 /* global scope is at level 0 */;
}
