#include "AST/return.hpp"

// TODO
ReturnNode::ReturnNode(const uint32_t line, const uint32_t col,
                       ExpressionNode *const p_expression)
    : AstNode{line, col}, m_expression(p_expression) {}

// TODO: You may use code snippets in AstDumper.cpp
void ReturnNode::print() {}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_expression->accept(p_visitor);
}
