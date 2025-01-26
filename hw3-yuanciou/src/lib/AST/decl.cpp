#include "AST/decl.hpp"

// TODO
DeclNode::DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode *> *const list_of_variable)
    : AstNode{line, col}, m_variables(*list_of_variable) {}

// TODO
//DeclNode::DeclNode(const uint32_t line, const uint32_t col)
//    : AstNode{line, col} {}

// TODO: You may use code snippets in AstDumper.cpp
void DeclNode::print() {}

void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    for (auto &variable : m_variables)
    {
        variable->accept(p_visitor);
    }
}

std::vector<const char *> DeclNode::declaration_Type() {
    std::vector<const char *> decl_type;

    for (auto &variable : m_variables)
    {
        const char * var_type = variable->get_variable_type();
        decl_type.push_back(var_type);
    }

    return decl_type;
}