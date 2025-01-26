#include "AST/read.hpp"

// TODO
ReadNode::ReadNode(const uint32_t line, const uint32_t col,
                   VariableReferenceNode *const p_variable_re)
    : AstNode{line, col}, m_variable_re(p_variable_re) {}

// TODO: You may use code snippets in AstDumper.cpp
void ReadNode::print() {}

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_variable_re->accept(p_visitor);
}