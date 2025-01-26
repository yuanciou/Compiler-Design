#include "AST/CompoundStatement.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/program.hpp"
#include "codegen/CodeGenerator.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <cstring>
#include<iostream>

CodeGenerator::CodeGenerator(const std::string &source_file_name,
                             const std::string &save_path,
                             std::unordered_map<SemanticAnalyzer::AstNodeAddr,
                                                      SymbolManager::Table>
                                 &&p_symbol_table_of_scoping_nodes)
    : m_symbol_manager(false /* no dump */),
      m_source_file_path(source_file_name),
      m_symbol_table_of_scoping_nodes(std::move(p_symbol_table_of_scoping_nodes)) {
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path =
        save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind('/');
    auto dot_pos = source_file_name.rfind('.');

    assert(dot_pos != std::string::npos && source_file_name[dot_pos+1]=='p' && "file not recognized");

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    } else {
        slash_pos = 0;
    }
    auto output_file_path{
        real_path + "/" +
        source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program) {
    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char *const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n";
        
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue,
                     m_source_file_path.c_str());

    // Reconstruct the scope for looking up the symbol entry.
    // Hint: Use m_symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_program)));

    cur_root.push_back("program");
    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(*this); };
    for_each(p_program.getDeclNodes().begin(), p_program.getDeclNodes().end(),
             visit_ast_node);
    for_each(p_program.getFuncNodes().begin(), p_program.getFuncNodes().end(),
             visit_ast_node);

    constexpr const char *const riscv_assembly_main = ".section    .text\n"
                                                      "    .align 2\n"
                                                      "    .globl main\n"
                                                      "    .type main, @function\nmain:\n";

    constexpr const char *const riscv_assembly_endmain = "    .size main, .-main\n";

    dumpInstructions(m_output_file.get(), riscv_assembly_main);

    const_cast<CompoundStatementNode &>(p_program.getBody()).accept(*this);
    cur_root.pop_back();

    m_symbol_manager.popScope();

    dumpInstructions(m_output_file.get(), riscv_assembly_endmain);
}

void CodeGenerator::visit(DeclNode &p_decl) {
    cur_root.push_back("Declaration");
    p_decl.visitChildNodes(*this);
    cur_root.pop_back();
}

void CodeGenerator::visit(VariableNode &p_variable) {
    cur_root.push_back("Variable");
    SymbolEntry *symbol_entry = const_cast<SymbolEntry*>(m_symbol_manager.lookup(p_variable.getName()));
    bool global = (symbol_entry->getLevel() == 0) ? true : false;
    SymbolEntry::KindEnum kind = symbol_entry->getKind();

    constexpr const char *const riscv_assembly_global_var = ".comm %s, 4, 4\n";
    constexpr const char *const riscv_assembly_global_const = ".section    .rodata\n"
                                                              "    .align 2\n"
                                                              "    .globl %s\n"
                                                              "    .type %s, @object\n%s:\n"
                                                              "    .word %s\n";
    constexpr const char *const riscv_assembly_local_var_t = "    sw t%d, -%d(s0)\n";    
    constexpr const char *const riscv_assembly_local_var_a = "    sw a%d, -%d(s0)\n";                                                    
    constexpr const char *const riscv_assembly_local_const_s = "    addi t0, s0, -%d\n"
                                                               "    addi sp, sp, -4\n"
                                                               "    sw t0, 0(sp)\n";
    constexpr const char *const riscv_assembly_local_const_e = "    lw t0, 0(sp)\n"
                                                               "    addi sp, sp, 4\n"
                                                               "    lw t1, 0(sp)\n"
                                                               "    addi sp, sp, 4\n"
                                                               "    sw t0, 0(t1)\n";
    
    if(global)
    {
        if (kind == SymbolEntry::KindEnum::kVariableKind) 
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_global_var, p_variable.getNameCString());
        }
        else if (kind == SymbolEntry::KindEnum::kConstantKind) 
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_global_const, p_variable.getNameCString(),p_variable.getNameCString(), p_variable.getNameCString(), symbol_entry->getAttribute().constant()->getConstantValueCString());
        }
    }
    else{
        // local variables
        if (kind == SymbolEntry::KindEnum::kVariableKind || kind == SymbolEntry::KindEnum::kParameterKind || kind == SymbolEntry::KindEnum::kLoopVarKind) 
        {
            cur_offset = cur_offset + 4;
            printf("para_in\n");
            symbol_entry->set_mem_offset(cur_offset);
            if (kind == SymbolEntry::KindEnum::kParameterKind)
            {
                if(p_count.func_para < 8)
                {
                    dumpInstructions(m_output_file.get(), riscv_assembly_local_var_a, ((cur_offset - 12) / 4) % 8, cur_offset);
                }
                else
                {
                    dumpInstructions(m_output_file.get(), riscv_assembly_local_var_t,  p_count.func_para % 8 + 2,  cur_offset);
                }
                p_count.func_para++;
            }
                
        }
        else if (kind == SymbolEntry::KindEnum::kConstantKind) 
        {
            cur_offset = cur_offset + 4;
            symbol_entry->set_mem_offset(cur_offset);
            dumpInstructions(m_output_file.get(), riscv_assembly_local_const_s, cur_offset);
            p_variable.visitChildNodes(*this);
            dumpInstructions(m_output_file.get(), riscv_assembly_local_const_e);
        }
    }
    cur_root.pop_back();

}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    cur_root.push_back("ConstantValue");
    constexpr const char *const riscv_assembly_constval = "    li t0, %s\n"
                                                          "    addi sp, sp, -4\n"
                                                          "    sw t0, 0(sp)\n";
    const char *value = p_constant_value.getConstantValueCString();
    std::string str_val = value;
    value = (str_val == "true") ? "1" : (str_val == "false") ? "0" : value;
    //printf(value);
    dumpInstructions(m_output_file.get(), riscv_assembly_constval, value);
    cur_root.pop_back();
}

void CodeGenerator::visit(FunctionNode &p_function) {
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
    std::move(m_symbol_table_of_scoping_nodes.at(&p_function)));
    
    constexpr const char *const riscv_assembly_func_s = "\n.section    .text\n"
                                                        "   .align 2\n"
                                                        "   .globl %s\n"
                                                        "   .type %s, @function\n\n%s:\n";
    constexpr const char *const riscv_assembly_func_pro = "    # in the function prologue\n"
                                                        "    addi sp, sp, -128\n"
                                                        "    sw ra, 124(sp)\n"
                                                        "    sw s0, 120(sp)\n"
                                                        "    addi s0, sp, 128\n\n";
    constexpr const char *const riscv_assembly_func_a =  "    sw a%d, -%d(s0)\n";
    constexpr const char *const riscv_assembly_func_t =  "    sw t%d, -%d(s0)\n";
    // constexpr const char *const riscv_assembly_func_epi = "\n    # in the function epilogue\n"
    //                                                 "    lw ra, 124(sp)\n"
    //                                                 "    lw s0, 120(sp)\n"
    //                                                 "    addi sp, sp, 128\n"
    //                                                 "    jr ra\n";
    constexpr const char *const riscv_assembly_func_e = "    .size %s, .-%s\n";
 
    dumpInstructions(m_output_file.get(), riscv_assembly_func_s, p_function.getNameCString(), p_function.getNameCString(), p_function.getNameCString());

    cur_offset = 8;

    dumpInstructions(m_output_file.get(), riscv_assembly_func_pro);

    FILE *file = m_output_file.get();
    const char *name = p_function.getNameCString();
    if (file == NULL) {
        fprintf(stderr, "Error: file pointer is NULL.\n");
        return;
    }
    if (name == NULL) {
        fprintf(stderr, "Error: name pointer is NULL.\n");
        return;
    }
    
    for(; p_count.func_para; p_count.func_para--)
    {
        if(p_count.func_para < 8)
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_func_a, ((cur_offset - 12) / 4) % 8, cur_offset);
        }
        else
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_func_t,  p_count.func_para % 8 + 2,  cur_offset);
        }
    }
  
    p_function.visitParameterChildNodes(*this);
    p_function.visitBodyChildNodes(*this);
    for(; p_count.func_para; p_count.func_para--)
    {
        cur_offset = cur_offset + 4;
        if(p_count.func_para < 8)
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_func_a, ((cur_offset - 12) / 4) % 8, cur_offset);
        }
        else
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_func_t,  p_count.func_para % 8 + 2,  cur_offset);
        }
    }

    //dumpInstructions(m_output_file.get(), riscv_assembly_func_epi);
    
    // Remove the entries in the hash table
    m_symbol_manager.popScope();
    
    
    dumpInstructions(m_output_file.get(), riscv_assembly_func_e, p_function.getNameCString(), p_function.getNameCString());
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
    std::move(m_symbol_table_of_scoping_nodes.at(&p_compound_statement)));

    cur_root.push_back("CompoundStatement");

    constexpr const char *const riscv_assembly_count = "L%d:";
    constexpr const char *const riscv_assembly_for = "    lw t0, 0(sp)\n"
                                                     "    addi sp, sp, 4\n"
                                                     "    lw t1, 0(sp)\n"
                                                     "    addi sp, sp, 4\n"
                                                     "    bge t1, t0, L%d\n"
                                                     "L%d:\n";
    constexpr const char *const riscv_assembly_func_prol = "    addi sp, sp, -128\n"
                                                           "    sw ra, 124(sp)\n"
                                                           "    sw s0, 120(sp)\n"
                                                           "    addi s0, sp, 128\n\n";
    constexpr const char *const riscv_assembly_func_para_a = "    sw a%d, -%d(s0)\n";
    constexpr const char *const riscv_assembly_func_para_t = "    sw t%d, -%d(s0)\n";
    constexpr const char *const riscv_assembly_count_end = "    j L%d\n";
    constexpr const char *const riscv_assembly_func_epil = "    lw ra, 124(sp)\n"
                                                           "    lw s0, 120(sp)\n"
                                                           "    addi sp, sp, 128\n"
                                                           "    jr ra\n";

    if(p_count.if_count || p_count.while_count)
    {
        dumpInstructions(m_output_file.get(), riscv_assembly_count, p_count.count++);
    }
    else if(p_count.for_count > 0)
    {
        dumpInstructions(m_output_file.get(), riscv_assembly_for, p_loop_manager.pseudo_count + 2, p_loop_manager.pseudo_count + 1);
    }
    else
    {
        dumpInstructions(m_output_file.get(), riscv_assembly_func_prol);
        cur_offset = 8;
        for(; p_count.func_para > 0; p_count.func_para--)
        {
            cur_offset = cur_offset + 4;
            if(p_count.func_para < 8)
            {
                dumpInstructions(m_output_file.get(), riscv_assembly_func_para_t,  p_count.func_para % 8 + 2 ,  cur_offset);
            }
            else
            {
                dumpInstructions(m_output_file.get(), riscv_assembly_func_para_a, ((cur_offset - 12) / 4) % 8, cur_offset);
            }
        }
    }

    p_compound_statement.visitChildNodes(*this);

    if (p_count.if_count > 0)
    {
        if (p_count.else_count > 0)
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_count_end, p_count.count + 1);
            p_count.else_count--;
        }
    }
    else if (p_count.while_count > 0)
    {
        dumpInstructions(m_output_file.get(), riscv_assembly_count_end, p_count.count - 2);
    }
    else if (p_count.for_count > 0) {}
    else 
    {
        dumpInstructions(m_output_file.get(), riscv_assembly_func_epil);
    }

    cur_root.pop_back();

    // Remove the entries in the hash table
     m_symbol_manager.popScope();
}

void CodeGenerator::visit(PrintNode &p_print) {
    constexpr const char *const riscv_assembly_print = "    lw a0, 0(sp)\n"
                                                       "    addi sp, sp, 4\n"
                                                       "    jal ra, printInt\n";

    cur_root.push_back("Print");
    p_print.visitChildNodes(*this);
    cur_root.pop_back();

    dumpInstructions(m_output_file.get(), riscv_assembly_print);
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    cur_root.push_back("BinaryOperator");

    p_count.op_count++;
    p_bin_op.visitChildNodes(*this);
    p_count.op_count--;

    constexpr const char *const riscv_assembly_binop_s = "    lw t0, 0(sp)\n"
                                                         "    addi sp, sp, 4\n"
                                                         "    lw t1, 0(sp)\n"
                                                         "    addi sp, sp, 4\n";
    constexpr const char *const riscv_assembly_binop_e = "    addi sp, sp, -4\n"
                                                         "    sw t0, 0(sp)\n";

    dumpInstructions(m_output_file.get(), riscv_assembly_binop_s);

    switch(p_bin_op.getOp())
    {
        case Operator::kDivideOp:
        {
            constexpr const char *const riscv_assembly_binop = "    div t0, t1, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop);
            dumpInstructions(m_output_file.get(), riscv_assembly_binop_e);
            break;
		}
        case Operator::kEqualOp:
        {
            constexpr const char *const riscv_assembly_binop = "    bne t1, t0, L%d\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop, p_count.count + 1);
            break;
		}
        case Operator::kGreaterOp:
        {
            constexpr const char *const riscv_assembly_binop = "    ble t1, t0, L%d\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop, p_count.count + 1);
            break;
		}
        case Operator::kGreaterOrEqualOp:
        {
            constexpr const char *const riscv_assembly_binop = "    blt t1, t0, L%d\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop, p_count.count + 1);
            break;
		}
        case Operator::kLessOp:
        {
            constexpr const char *const riscv_assembly_binop = "    bge t1, t0, L%d\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop, p_count.count + 1);
            break;
		}
        case Operator::kLessOrEqualOp:
        {
            constexpr const char *const riscv_assembly_binop = "    bgt t1, t0, L%d\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop, p_count.count + 1);
            break;
		}
        case Operator::kMinusOp:
        {
            constexpr const char *const riscv_assembly_binop = "    sub t0, t1, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop);
            dumpInstructions(m_output_file.get(), riscv_assembly_binop_e);
            break;
		}
        case Operator::kModOp:
        {
            constexpr const char *const riscv_assembly_binop = "    rem t0, t1, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop);
            dumpInstructions(m_output_file.get(), riscv_assembly_binop_e);
            break;
		}
        case Operator::kMultiplyOp:
        {
            constexpr const char *const riscv_assembly_binop = "    mul t0, t1, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop);
            dumpInstructions(m_output_file.get(), riscv_assembly_binop_e);
            break;
		}
        case Operator::kNotEqualOp:
        {
            constexpr const char *const riscv_assembly_binop = "    beq t1, t0, L%d\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop, p_count.count + 1);
            break;
		}
        case Operator::kPlusOp:
        {
            constexpr const char *const riscv_assembly_binop = "    add t0, t1, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop);
            dumpInstructions(m_output_file.get(), riscv_assembly_binop_e);
            break;
		}
        case Operator::kAndOp:
        {
            constexpr const char *const riscv_assembly_binop = "    and t0, t1, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop);
            dumpInstructions(m_output_file.get(), riscv_assembly_binop_e);
            break;
        }
        case Operator::kOrOp:
        {
            constexpr const char *const riscv_assembly_binop = "    or t0, t1, t0";
            dumpInstructions(m_output_file.get(), riscv_assembly_binop);
            dumpInstructions(m_output_file.get(), riscv_assembly_binop_e);
            break;
        }
    }

    cur_root.pop_back();
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    cur_root.push_back("Un_op");
	p_un_op.visitChildNodes(*this);
	cur_root.pop_back();
    constexpr const char *const riscv_assembly_unop_s = "    lw t0, 0(sp)\n"
                                                        "    addi sp, sp, 4\n";
    constexpr const char *const riscv_assembly_unop_e = "    addi sp, sp, -4\n"
                                                        "    sw t0, 0(sp)\n";

    dumpInstructions(m_output_file.get(), riscv_assembly_unop_s);
    switch(p_un_op.getOp())
    {
        case Operator::kNegOp:
        {
            constexpr const char *const riscv_assembly_unop = "    neg t0, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_unop);
            dumpInstructions(m_output_file.get(), riscv_assembly_unop_e);
            break;
        }
        case Operator::kNotOp:
        {
            constexpr const char *const riscv_assembly_unop = "    li t1, -1\n"
					                                          "    add t0, t0, t1\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_unop);
            dumpInstructions(m_output_file.get(), riscv_assembly_unop_e);
            break;
        }
    }
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation)
{
    cur_root.push_back("Function_Invocation");

    constexpr const char *const riscv_assembly_invo_a =  "    lw a%d, 0(sp)\n"
                                                         "    addi sp, sp, 4\n";
    constexpr const char *const riscv_assembly_invo_t =  "    lw t%d, 0(sp)\n"
                                                         "    addi sp, sp, 4\n";           
    constexpr const char *const riscv_assembly_invo_e = "    jal ra, %s\n"
                                                        "    mv t0, a0\n"
                                                        "    addi sp, sp, -4\n"
                                                        "    sw t0, 0(sp)\n";                                    

    p_count.func_invo_count++;
    p_func_invocation.visitChildNodes(*this);
    p_count.func_invo_count--;

    for (int i = 0; i < p_func_invocation.getArguments().size(); i++){
        if(i < 8)
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_invo_a, i);
        }
        else{
            dumpInstructions(m_output_file.get(), riscv_assembly_invo_t, i % 8 + 2);
        }
    }
    dumpInstructions(m_output_file.get(), riscv_assembly_invo_e, p_func_invocation.getNameCString());
    cur_root.pop_back();
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    cur_root.push_back("VariableRef");
    if (SymbolEntry *symbol_entry = m_symbol_manager.lookup(p_variable_ref.getName())) 
    {
        bool global = (symbol_entry->getLevel() == 0) ? true : false;
        int current_offset = symbol_entry->get_mem_offset();
        SymbolEntry::KindEnum kind = symbol_entry->getKind();
        constexpr const char *const riscv_assembly_local_1 = "    addi t0, s0, -%d\n"
                                                         "    addi sp, sp, -4\n"
                                                         "    sw t0, 0(sp)\n";
        constexpr const char *const riscv_assembly_local_2 = "    lw t0, -%d(s0)\n"
                                                            "    addi sp, sp, -4\n"
                                                            "    sw t0, 0(sp)\n";
        constexpr const char *const riscv_assembly_global_1 = "    la t0, %s\n"
                                                            "    addi sp, sp, -4\n"
                                                            "    sw t0, 0(sp)\n";
        constexpr const char *const riscv_assembly_global_2 = "    la t0, %s\n"
                                                            "    lw t1, 0(t0)\n"
                                                            "    mv t0, t1\n"
                                                            "    addi sp, sp, -4\n"
                                                            "    sw t0, 0(sp)\n";
        
        if(kind == SymbolEntry::KindEnum::kParameterKind)
        {
            dumpInstructions(m_output_file.get(), riscv_assembly_local_2, current_offset);
        }
        else if(global)
        {
            if(kind == SymbolEntry::KindEnum::kVariableKind || kind == SymbolEntry::KindEnum::kConstantKind)
            {
                if ((p_status.assign_left == true || p_status.read == true) && !p_count.op_count && !p_count.func_invo_count) 
                {
                    dumpInstructions(m_output_file.get(), riscv_assembly_global_1, symbol_entry->getNameCString());
                }
                else 
                {
                    dumpInstructions(m_output_file.get(), riscv_assembly_global_2, symbol_entry->getNameCString());
                }
            }
        }
        else
        {
            if(kind == SymbolEntry::KindEnum::kVariableKind || kind == SymbolEntry::KindEnum::kConstantKind || kind == SymbolEntry::KindEnum::kLoopVarKind)
            {
                if ((p_status.assign_left == true || p_status.read == true) && !p_count.op_count && !p_count.func_invo_count) 
                {
                    dumpInstructions(m_output_file.get(), riscv_assembly_local_1, current_offset);
                }
                else 
                {
                    dumpInstructions(m_output_file.get(), riscv_assembly_local_2, current_offset);
                }
            }
        }
    }
    cur_root.pop_back();
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    cur_root.push_back("Assignment");

    constexpr const char *const riscv_assembly_assignment = "    lw t0, 0(sp)\n"
                                                            "    addi sp, sp, 4\n"
                                                            "    lw t1, 0(sp)\n"
                                                            "    addi sp, sp, 4\n"
                                                            "    sw t0, 0(t1)    # save the value\n";
    constexpr const char *const riscv_assembly_for_assignment = "L%d:\n"
                                                                "    lw t0, -%d(s0)\n"
                                                                "    addi sp, sp, -4\n"
                                                                "    sw t0, 0(sp)\n";
    
    p_status.assign_left = true;
    p_assignment.visitChildNodes(*this);
    p_status.assign_left = false;

    dumpInstructions(m_output_file.get(), riscv_assembly_assignment);

    /*for(; p_count.for_assign_count > 0; p_count.for_assign_count--)
    {
        dumpInstructions(m_output_file.get(), riscv_assembly_for_assignment, p_loop_manager.pseudo_count, cur_offset);
    }*/
    if (p_count.for_assign_count) {
        dumpInstructions(m_output_file.get(), "L%d:\n"
                                             "    lw t0, -%d(s0)\n"
                                             "    addi sp, sp, -4\n"
                                             "    sw t0, 0(sp)\n", p_loop_manager.pseudo_count, cur_offset);
        p_count.for_assign_count--;
    }

    cur_root.pop_back();
}

void CodeGenerator::visit(ReadNode &p_read) {
    cur_root.push_back("Read");
    p_status.read = true;
    p_read.visitChildNodes(*this);
    p_status.read = false;

    constexpr const char *const riscv_assembly_read = "    jal ra, readInt\n"
                                                      "    lw t0, 0(sp)\n"
                                                      "    addi sp, sp, 4\n"
                                                      "    sw a0, 0(t0)\n";
    dumpInstructions(m_output_file.get(), riscv_assembly_read);
    cur_root.pop_back();
}

void CodeGenerator::visit(IfNode &p_if) {
    cur_root.push_back("If");
    constexpr const char *const riscv_assembly_if = "L%d:";

    p_count.if_count++;
    if(p_if.else_body() == true)
    {
        p_count.else_count = p_count.else_count + 1;
    }
    p_if.visitChildNodes(*this);
    p_count.if_count--;

    dumpInstructions(m_output_file.get(), riscv_assembly_if, p_count.count++);
    cur_root.pop_back();
}

void CodeGenerator::visit(WhileNode &p_while) {
    cur_root.push_back("While");
    constexpr const char *const riscv_assembly_while = "\nL%d:\n";
    
    dumpInstructions(m_output_file.get(), riscv_assembly_while, p_count.count++);

    p_count.while_count++;
    p_while.visitChildNodes(*this);
    p_count.while_count--;

    dumpInstructions(m_output_file.get(), riscv_assembly_while, p_count.count++);
    cur_root.pop_back();
}

void CodeGenerator::visit(ForNode &p_for) {
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_for)));

    cur_root.push_back("For");
    constexpr const char *const riscv_assembly_for = "    addi t0, s0, -%d\n"
                                                    "    addi sp, sp, -4\n"
                                                    "    sw t0, 0(sp)\n"
                                                    "    lw t0, -%d(s0)\n"
                                                    "    addi sp, sp, -4\n"
                                                    "    sw t0, 0(sp)\n"
                                                    "    li t0, 1\n"
                                                    "    addi sp, sp, -4\n"
                                                    "    sw t0, 0(sp)\n"
                                                    "    lw t0, 0(sp)\n"
                                                    "    addi sp, sp, 4\n"
                                                    "    lw t1, 0(sp)\n"
                                                    "    addi sp, sp, 4\n"
                                                    "    add t0, t1, t0\n"
                                                    "    addi sp, sp, -4\n"
                                                    "    sw t0, 0(sp)\n"
                                                    "    lw t0, 0(sp)\n"
                                                    "    addi sp, sp, 4\n"
                                                    "    lw t1, 0(sp)\n"
                                                    "    addi sp, sp, 4\n"
                                                    "    sw t0, 0(t1)\n"
                                                    "    j L%d\n"
                                                    "L%d:\n";
    
    p_loop_manager.pseudo_count = p_count.count;
    p_loop_manager.count_loop.push_back(p_count.count);
    p_count.count = p_count.count + 3;
    p_count.for_count++;
    p_count.for_assign_count++;
    p_for.visitChildNodes(*this);
    p_count.for_count--;
    
    dumpInstructions(m_output_file.get(), riscv_assembly_for, cur_offset, cur_offset, p_loop_manager.pseudo_count, p_loop_manager.pseudo_count + 2);
    // Remove the entries in the hash table
    m_symbol_manager.popScope();
    cur_offset = cur_offset - 4;
    p_loop_manager.count_loop.pop_back();
    if(p_loop_manager.count_loop.size() == 0)
    {
        p_loop_manager.pseudo_count  = 0;
    }
    else
    {
        p_loop_manager.pseudo_count = p_loop_manager.count_loop.back();
    }
    cur_root.pop_back();
}

void CodeGenerator::visit(ReturnNode &p_return) {
    cur_root.push_back("Return");
    p_return.visitChildNodes(*this);

    constexpr const char *const riscv_assembly_return = "    lw a0, 0(sp)\n"
                                                        "    addi sp, sp, 4\n"
                                                        "    mv a0, t0\n"
                                                        "\n    # in the function epilogue\n"
                                                        "    lw ra, 124(sp)\n"
                                                        "    lw s0, 120(sp)\n"
                                                        "    addi sp, sp, 128\n"
                                                        "    jr ra\n";
    // constexpr const char *const riscv_assembly_func_epi = "\n    # in the function epilogue\n"
    //                                                 "    lw ra, 124(sp)\n"
    //                                                 "    lw s0, 120(sp)\n"
    //                                                 "    addi sp, sp, 128\n"
    //                                                 "    jr ra\n";
    dumpInstructions(m_output_file.get(), riscv_assembly_return);
    //dumpInstructions(m_output_file.get(), riscv_assembly_func_epi);
    cur_root.pop_back();
}
