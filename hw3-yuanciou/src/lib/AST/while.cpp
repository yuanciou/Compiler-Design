#include "AST/while.hpp"

// TODO
WhileNode::WhileNode(const uint32_t line, const uint32_t col,
                     ExpressionNode *const p_expression, CompoundStatementNode *const p_compound_s)
    : AstNode{line, col}, m_expression(p_expression), m_compound_s(p_compound_s) {}

// TODO: You may use code snippets in AstDumper.cpp
void WhileNode::print() {}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_expression->accept(p_visitor);
    m_compound_s->accept(p_visitor);
}
