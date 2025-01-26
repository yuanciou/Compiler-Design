#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/CompoundStatement.hpp"
#include "visitor/AstNodeVisitor.hpp"

class FunctionNode : public AstNode {
  public:
    FunctionNode(const uint32_t line, const uint32_t col,
                 /* TODO: name, declarations, return type,
                  *       compound statement (optional) */
                 std::string p_name, std::vector<DeclNode *> *const p_declarations,
                 std::string p_return_type, CompoundStatementNode *p_compound_s);
    ~FunctionNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *get_function_name() { return m_name.c_str(); };
    const char *get_function_parameter_type() { return m_parameter_type.c_str(); };

  private:
    // TODO: name, declarations, return type, compound statement
    std::string m_name;
    std::vector<DeclNode *> *m_declaration;
    std::string m_return_type, m_parameter_type;
    CompoundStatementNode *m_compound_s;
};

#endif
