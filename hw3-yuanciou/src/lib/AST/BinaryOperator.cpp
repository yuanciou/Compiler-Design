#include "AST/BinaryOperator.hpp"

// TODO
BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col,
                                       std::string p_operator, ExpressionNode *const p_left, ExpressionNode *const p_rignt)
    : ExpressionNode{line, col}, m_operator(p_operator), m_left(p_left), m_right(p_rignt) {}

// TODO: You may use code snippets in AstDumper.cpp
void BinaryOperatorNode::print() {}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_left->accept(p_visitor);
    m_right->accept(p_visitor);
}
