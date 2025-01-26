#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ast.hpp"
#include "AST/ConstantValue.hpp"
#include "visitor/AstNodeVisitor.hpp"

class VariableNode : public AstNode {
  public:
    VariableNode(const uint32_t line, const uint32_t col,
                 /* TODO: variable name, type, constant value */
                 const char *p_var_name, const char *p_var_type, ConstantValueNode *p_const_value);

    ~VariableNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *get_variable_name() { return m_var_name.c_str(); }
    const char *get_variable_type() { return m_var_type.c_str(); }

  private:
    // TODO: variable name, type, constant value
    std::string m_var_name;
    std::string m_var_type;
    ConstantValueNode* m_const_value;
};

#endif
