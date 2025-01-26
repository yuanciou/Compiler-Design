#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "sema/ErrorPrinter.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <cstdint>
#include <cstdio>
#include <set>
#include <stack>
#include <unordered_map>

class SemanticAnalyzer final : public AstNodeVisitor {
  public:
    using AstNodeAddr = const AstNode *;

  private:
    enum class SemanticContext : uint8_t {
        kGlobal,
        kFunction,
        kForLoop,
        kLocal
    };

  private:
    SymbolManager m_symbol_manager;
    /// @brief Four kinds of AST nodes opens a scope: program, function, loop, and
    /// compound statement. The symbol table of the scope they opened is stored
    /// and mapped back to the AST node. This is for the scope structure to be
    /// reconstructed while generating code.
    std::unordered_map<AstNodeAddr, SymbolManager::Table>
        m_symbol_table_of_scoping_nodes;
    std::stack<SemanticContext> m_context_stack;
    std::stack<const PType *> m_returned_type_stack;

    std::set<SymbolEntry *> m_error_entry_set;

    bool m_has_error = false;
    ErrorPrinter m_error_printer;

  public:
    /// @return The symbol table of the AST nodes that open a scope: program,
    /// function, loop, and compound statement. This is for the scope structure
    /// to be reconstructed while generating code.
    /// @note This function is called after the semantic analysis is done and can
    /// only be called once.
    std::unordered_map<AstNodeAddr, SymbolManager::Table> &&
    acquireSymbolTableOfScopingNodes() {
        return std::move(m_symbol_table_of_scoping_nodes);
    }

    ~SemanticAnalyzer() = default;
    SemanticAnalyzer(const bool p_opt_dmp, std::FILE *p_error_stream = stderr)
        : m_symbol_manager(p_opt_dmp), m_error_printer(p_error_stream) {}

    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;

    bool hasError() const { return m_has_error; }

  private:
    /// @brief Prints the error and sets the error flag to `true`.
    /// @note Call this function instead of using the error printer directly.
    void printError(const Error&);
    /// @brief In addition to printing `printError`, the type of the expression
    /// is set as `kErrorType`.
    /// @note This is a convenience function to prevent from forgetting to set the
    /// type on error.
    void printErrorAndSetType(const Error&, ExpressionNode&);

    SymbolEntry::KindEnum determineVarKind(
        const VariableNode &p_var_node) const;

    bool isShadowingLoopVar(const std::string& p_name) const;
    bool isRedeclaringSymbol(const std::string& p_name) const;

    /// @note Since reporting errors on arguments requires information specific
    /// to such arguments, we report errors inside this function.
    bool analyzeArgumentTypes(
        const FunctionNode::DeclNodes &p_parameters,
        const FunctionInvocationNode::ExprNodes &p_arguments);
    /// @note Since there are multiple kinds of errors that can be reported on
    /// the lvalue, we report errors inside this function.
    bool analyzeAssignmentLvalue(const AssignmentNode &p_assignment,
                                  const SymbolManager &p_symbol_manager);
    /// @note Since there are multiple kinds of errors that can be reported on
    /// the expression, we report errors inside this function.
    bool analyzeAssignmentExpr(const AssignmentNode &p_assignment);
};

#endif
