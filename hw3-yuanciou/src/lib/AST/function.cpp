#include "AST/function.hpp"

// TODO
FunctionNode::FunctionNode(const uint32_t line, const uint32_t col,
                           std::string p_name, std::vector<DeclNode *> *const p_declarations,
                           std::string p_return_type, CompoundStatementNode *p_compound_s)
    : AstNode{line, col}, m_name(p_name), m_declaration(p_declarations), m_return_type(p_return_type), m_compound_s(p_compound_s) {
        std::string tmp_variable_type;
        if (m_declaration != nullptr) {
            for (int i = 0; i < m_declaration->size() - 1; i++)
            {
                std::vector<const char *> all_parameter_type = m_declaration->at(i)->declaration_Type();
                for (int j = 0; j < all_parameter_type.size() - 1; j++)
                {
                    tmp_variable_type = tmp_variable_type + all_parameter_type[j] + ", ";
                }
                tmp_variable_type = tmp_variable_type + all_parameter_type[all_parameter_type.size() - 1] + ", ";
            }
            std::vector<const char *> all_parameter_type = m_declaration->at(m_declaration->size() - 1)->declaration_Type();
            for (int j = 0; j < all_parameter_type.size() - 1; j++)
                {
                    tmp_variable_type = tmp_variable_type + all_parameter_type[j] + ", ";
                }
                tmp_variable_type = tmp_variable_type + all_parameter_type[all_parameter_type.size() - 1];
        }
        m_parameter_type = p_return_type + " (" + tmp_variable_type + ')';
    }

// TODO: You may use code snippets in AstDumper.cpp
void FunctionNode::print() {}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if (m_declaration != nullptr) {
        for (auto &decl : *m_declaration) {
            decl->accept(p_visitor);
        }
    }

    if (m_compound_s != nullptr)
        m_compound_s->accept(p_visitor);
}
