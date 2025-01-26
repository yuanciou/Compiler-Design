#ifndef SEMA_SYMBOL_TABLE_H
#define SEMA_SYMBOL_TABLE_H

#include "AST/constant.hpp"
#include "AST/PType.hpp"
#include "AST/function.hpp"

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

/*
 * Conform to C++ Core Guidelines C.182
 */
class Attribute {
  private:
    enum class Tag { kConstantValue, kParameterDeclNodes };
    Tag m_type;

    union {
        // raw pointer, does not own the object
        const Constant *m_constant_value_ptr;
        const FunctionNode::DeclNodes *m_parameters_ptr;
    };

  public:
    ~Attribute() = default;

    Attribute(const Constant *p_constant)
        : m_type(Tag::kConstantValue), m_constant_value_ptr(p_constant) {}

    Attribute(const FunctionNode::DeclNodes *p_parameters)
        : m_type(Tag::kParameterDeclNodes), m_parameters_ptr(p_parameters) {}

    const Constant *constant() const;
    const FunctionNode::DeclNodes *parameters() const;
};

class SymbolEntry {
  public:
    enum class KindEnum : uint8_t {
        kProgramKind,
        kFunctionKind,
        kParameterKind,
        kVariableKind,
        kLoopVarKind,
        kConstantKind
    };

  private:
    const std::string &m_name;
    KindEnum m_kind;
    size_t m_level;
    const PType *m_p_type;
    Attribute m_attribute;

    int mem_offset = -1;

  public:
    ~SymbolEntry() = default;

    SymbolEntry(const std::string &p_name, const KindEnum p_kind,
                const size_t p_level, const PType *const p_p_type,
                const Constant *const p_constant)
        : m_name(p_name), m_kind(p_kind), m_level(p_level), m_p_type(p_p_type),
          m_attribute(p_constant) {}

    SymbolEntry(const std::string &p_name, const KindEnum p_kind,
                const size_t p_level, const PType *const p_p_type,
                const FunctionNode::DeclNodes *const p_parameters)
        : m_name(p_name), m_kind(p_kind), m_level(p_level), m_p_type(p_p_type),
          m_attribute(p_parameters) {}

    const std::string &getName() const { return m_name; }
    const char *getNameCString() const { return m_name.c_str(); }

    const KindEnum getKind() const { return m_kind; }

    const size_t getLevel() const { return m_level; }

    const PType *getTypePtr() const { return m_p_type; }

    const Attribute &getAttribute() const { return m_attribute; }

    void set_mem_offset(int value) { this->mem_offset = value; }
    int get_mem_offset() { return mem_offset; }
};

class SymbolTable {
  private:
    std::vector<std::unique_ptr<SymbolEntry>> m_entries;

  public:
    ~SymbolTable() = default;
    SymbolTable() = default;

    /// @return `nullptr` if not found.
    SymbolEntry *lookup(const std::string &p_name) const;

    SymbolEntry *addSymbol(const std::string &p_name,
                           const SymbolEntry::KindEnum p_kind, const size_t p_level,
                           const PType *const p_p_type,
                           const Constant *const p_constant);
    SymbolEntry *addSymbol(const std::string &p_name,
                           const SymbolEntry::KindEnum p_kind, const size_t p_level,
                           const PType *const p_p_type,
                           const FunctionNode::DeclNodes *const p_parameters);
    void dump() const;
};

class SymbolManager {
  public:
    using Table = std::unique_ptr<SymbolTable>;

  private:
    std::vector<Table> m_tables;

    const bool m_opt_dmp;

  public:
    ~SymbolManager() = default;
    SymbolManager(const bool p_opt_dmp) : m_opt_dmp(p_opt_dmp) {}

    // initial construction
    void pushScope();
    Table popScope();
    /// @brief Pushes the given table as the new scope.
    void pushScope(Table p_table);

    /// @tparam AttributeType `Constant` or `FunctionNode::DeclNodes`
    /// @return The entry of the added symbol; `nullptr` if already exists in
    /// the current scope.
    /// @note Knows nothing about special shadowing rules, such as the shadowing
    /// of loop variables. The caller should handle them.
    template <typename AttributeType>
    SymbolEntry *addSymbol(const std::string &p_name,
                           const SymbolEntry::KindEnum p_kind,
                           const PType *const p_p_type,
                           const AttributeType *const p_attribute);

    /// @brief Looks up the symbol from the current table to the global table.
    /// @param p_name
    /// @return `nullptr` if not found.
    SymbolEntry *lookup(const std::string &p_name) const;

    /// @return `nullptr` if no scope is pushed.
    const SymbolTable *getCurrentTable() const;

    /// @note Overflows if no scope is pushed.
    size_t getCurrentLevel() const;
};

#endif
