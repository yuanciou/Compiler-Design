#ifndef CODEGEN_CODE_GENERATOR_H
#define CODEGEN_CODE_GENERATOR_H

#include "sema/SemanticAnalyzer.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>

class CodeGenerator final : public AstNodeVisitor {
  private:
    SymbolManager m_symbol_manager;
    std::string m_source_file_path;
    std::unordered_map<SemanticAnalyzer::AstNodeAddr,
                             SymbolManager::Table>
        m_symbol_table_of_scoping_nodes;
    /// NOTE: `FILE` cannot be simply deleted by `delete`, so we need a custom deleter.
    std::unique_ptr<FILE, decltype(&fclose)> m_output_file{nullptr, &fclose};
    int test;
    struct Count
    {
      int func_para = 0;
      int count = 1;
      int if_count = 0, else_count = 0, while_count = 0, for_count = 0, op_count = 0, func_invo_count = 0, for_assign_count = 0;
    };

    struct Status
    {
      bool assign_left = false;
      bool read = false;
    };

    struct Inner_Loop
    {
      std::vector<int> count_loop;
      int pseudo_count = 0;
    };

    std::vector<std::string> cur_root;
    int cur_offset = 8;
    Count p_count;
    Status p_status;
    Inner_Loop p_loop_manager;

  public:
    ~CodeGenerator() = default;
    CodeGenerator(const std::string &source_file_name,
                  const std::string &save_path,
                  std::unordered_map<SemanticAnalyzer::AstNodeAddr,
                                           SymbolManager::Table>
                      &&p_symbol_table_of_scoping_nodes);
    
    void addrStackPush(const std::string p_name);
    void addrStackPop(const std::string p_name);

    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;
};

#endif
