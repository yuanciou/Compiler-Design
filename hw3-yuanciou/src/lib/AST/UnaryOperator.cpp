#include "AST/UnaryOperator.hpp"

// TODO
UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col,
                                     std::string p_operator, ExpressionNode *const p_expression)
    : ExpressionNode{line, col}, m_operator(p_operator), m_expression(p_expression) {}

// TODO: You may use code snippets in AstDumper.cpp
void UnaryOperatorNode::print() {}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_expression->accept(p_visitor);
}
