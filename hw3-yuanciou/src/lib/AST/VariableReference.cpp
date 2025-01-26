#include "AST/VariableReference.hpp"

// TODO
VariableReferenceNode::VariableReferenceNode(const uint32_t line,
                                             const uint32_t col,
                                             std::string p_name, std::vector<ExpressionNode *> *const p_expressions)
    : ExpressionNode{line, col}, m_var_re_name(p_name), m_expression(p_expressions) {}

// TODO
// VariableReferenceNode::VariableReferenceNode(const uint32_t line,
//                                              const uint32_t col)
//     : ExpressionNode{line, col} {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableReferenceNode::print() {}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if (m_expression != nullptr)
    {
        for (auto &expression : *m_expression)
        {
            expression->accept(p_visitor);
        }
    }
    
}
