#include "AST/if.hpp"

// TODO
IfNode::IfNode(const uint32_t line, const uint32_t col,
               ExpressionNode *const p_expression, CompoundStatementNode *const p_compound_s1, CompoundStatementNode *const p_compound_s2)
    : AstNode{line, col}, m_expression(p_expression), m_compound_s1(p_compound_s1), m_compound_s2(p_compound_s2) {}

// TODO: You may use code snippets in AstDumper.cpp
void IfNode::print() {}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_expression->accept(p_visitor);
    m_compound_s1->accept(p_visitor);
    //else
    if (m_compound_s2 != nullptr)
    {
        m_compound_s2->accept(p_visitor);
    }
}
