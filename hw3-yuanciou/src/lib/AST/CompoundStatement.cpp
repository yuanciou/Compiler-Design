#include "AST/CompoundStatement.hpp"

// TODO
CompoundStatementNode::CompoundStatementNode(const uint32_t line,
                                             const uint32_t col,
                                             std::vector<DeclNode *> *const p_declarations,
                                             std::vector<AstNode *> *const p_statement)
    : AstNode{line, col}, m_declaration(p_declarations), m_statement(p_statement) {}

// TODO: You may use code snippets in AstDumper.cpp
void CompoundStatementNode::print() {}

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
    if (m_declaration != nullptr) 
    {
        for (auto &declaration : *m_declaration)
        {
            declaration->accept(p_visitor);
        }
    }

    if (m_statement != nullptr) 
    {
        for (auto &statement : *m_statement) 
        {
            statement->accept(p_visitor);
        }
    }
}
