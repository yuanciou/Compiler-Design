#ifndef AST_ASSIGNMENT_NODE_H
#define AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "AST/VariableReference.hpp"

#include <memory>

class AssignmentNode final : public AstNode {
  private:
    std::unique_ptr<VariableReferenceNode> m_lvalue;
    std::unique_ptr<ExpressionNode> m_expr;

  public:
    ~AssignmentNode() = default;
    AssignmentNode(const uint32_t line, const uint32_t col,
                   VariableReferenceNode *p_var_ref, ExpressionNode *p_expr)
        : AstNode{line, col}, m_lvalue(p_var_ref), m_expr(p_expr){}

    VariableReferenceNode *getVarNode() { return &*m_lvalue; }
    ExpressionNode *getExprNode() { return &*m_expr; }

    int getVarDim() { return (m_lvalue->type_ptr->getDimensionsSize() - m_lvalue->getSizeOfDimension()); }
    int getExprDim() { return (m_expr->type_ptr->getDimensionsSize()- m_expr->getSizeOfDimension()); }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
