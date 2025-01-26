#ifndef __AST_RETURN_NODE_H
#define __AST_RETURN_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

class ReturnNode : public AstNode {
  public:
    ReturnNode(const uint32_t line, const uint32_t col,
               /* TODO: expression */
               ExpressionNode *const p_expression);
    ~ReturnNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: expression
    ExpressionNode *m_expression;
};

#endif
