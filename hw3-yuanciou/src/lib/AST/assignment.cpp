#include "AST/assignment.hpp"

// TODO
AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col,
                               VariableReferenceNode *const p_variable_re, ExpressionNode *const p_expression)
    : AstNode{line, col}, m_variable_re(p_variable_re), m_expression(p_expression) {}

// TODO: You may use code snippets in AstDumper.cpp
void AssignmentNode::print() {}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_variable_re->accept(p_visitor);
    m_expression->accept(p_visitor);
}
