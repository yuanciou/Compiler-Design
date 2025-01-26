#include "AST/variable.hpp"

// TODO
VariableNode::VariableNode(const uint32_t line, const uint32_t col, 
                           const char *p_var_name, const char *p_var_type, ConstantValueNode *p_const_value)
    : AstNode{line, col}, m_var_name(p_var_name), m_var_type(p_var_type), m_const_value(p_const_value) {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableNode::print() {}

void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if (m_const_value != nullptr)
    {
        m_const_value->accept(p_visitor);
    }
}
