#include "AST/program.hpp"

// TODO
ProgramNode::ProgramNode(const uint32_t line, const uint32_t col,
                         const char *const p_name, 
                         std::vector<DeclNode *> *const p_declaraion, 
                         std::vector<FunctionNode *> *const p_function, CompoundStatementNode *const p_body)
    : AstNode{line, col}, name(p_name), m_declaration(p_declaraion), m_function(p_function), m_body(p_body) {}

// visitor pattern version: const char *ProgramNode::getNameCString() const { return name.c_str(); }

void ProgramNode::print() {
    // TODO
    // outputIndentationSpace();

    std::printf("program <line: %u, col: %u> %s %s\n",
                location.line, location.col,
                name.c_str(), "void");

    // TODO
    // incrementIndentation();
    // visitChildNodes();
    // decrementIndentation();
}


void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) { // visitor pattern version
//     /* TODO
//      *
    if (m_declaration != nullptr)
    {
        for (auto &decl : *m_declaration) {
            decl->accept(p_visitor);
        }
    }

    // functions
    if (m_function != nullptr)
    {
       for (auto &function : *m_function) {
            function->accept(p_visitor);
        } 
    }
//      * for (auto &decl : var_decls) {
//      *     decl->accept(p_visitor);
//      * }
//      *
//      * // functions
//      *
    m_body->accept(p_visitor);
//      */
}
