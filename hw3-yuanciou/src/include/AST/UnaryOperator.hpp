#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <string>

class UnaryOperatorNode : public ExpressionNode {
  public:
    UnaryOperatorNode(const uint32_t line, const uint32_t col,
                      /* TODO: operator, expression */
                      std::string p_operator, ExpressionNode *const p_expression);
    ~UnaryOperatorNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *get_operation() { return m_operator.c_str(); };

  private:
    // TODO: operator, expression
    std::string m_operator;
    ExpressionNode *m_expression;
};

#endif
