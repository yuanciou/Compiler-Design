#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/ast.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/decl.hpp"
#include "AST/function.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <string>
#include <memory>

class ProgramNode final : public AstNode {
  private:
    std::string name;
    std::vector<DeclNode *> *m_declaration;
    std::vector<FunctionNode *> *m_function;
    CompoundStatementNode *m_body;
    // TODO: return type, declarations, functions, compound statement
    

  public:
    ~ProgramNode() = default;
    ProgramNode(const uint32_t line, const uint32_t col, const char *const p_name, 
                /* TODO: return type, declarations, functions,
                 *       compound statement */
                std::vector<DeclNode *> *const p_declaration, 
                std::vector<FunctionNode *> *p_function, CompoundStatementNode *const p_body);

    // visitor pattern version: const char *getNameCString() const;
    void print() override;
    const char *getNameCString() const { return name.c_str(); }
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
