#include "AST/FunctionInvocation.hpp"

// TODO
FunctionInvocationNode::FunctionInvocationNode(const uint32_t line,
                                               const uint32_t col,
                                               std::string p_function_name, std::vector<ExpressionNode *> *const p_expressions)
    : ExpressionNode{line, col}, m_name(p_function_name), m_expression(p_expressions) {}

// TODO: You may use code snippets in AstDumper.cpp
void FunctionInvocationNode::print() {}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if (m_expression != nullptr) {
        for (auto &expression : *m_expression) {
            expression->accept(p_visitor);
        }
    }
}
