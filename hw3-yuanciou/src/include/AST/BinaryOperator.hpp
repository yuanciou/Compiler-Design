#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>

class BinaryOperatorNode : public ExpressionNode {
  public:
    BinaryOperatorNode(const uint32_t line, const uint32_t col,
                       /* TODO: operator, expressions */
                       std::string p_operator, ExpressionNode *const p_left, ExpressionNode *const p_rignt);
    ~BinaryOperatorNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *get_operation() { return m_operator.c_str(); };

  private:
    // TODO: operator, expressions
    std::string m_operator;
    ExpressionNode *m_left;
    ExpressionNode *m_right;
};

#endif
