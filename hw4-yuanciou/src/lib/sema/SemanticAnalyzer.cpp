#include "sema/Error.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"

using namespace std;

extern int32_t opt_dump;

void dumpDemarcation(const char chr) {
  for (size_t i = 0; i < 110; ++i) {
    printf("%c", chr);
  }
  puts("");
}

void dumpSymbol(SymbolTable *symbol_table) {
//   if (!symbol_table) {
//         printf("Symbol table is null\n");
//         return;
//     }
//     else {printf("hihi\n");}
  dumpDemarcation('=');
  printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
                                        "Attribute");
  dumpDemarcation('-');
  {
    symbol_table->PrintTable();
  }
  dumpDemarcation('-');
}

bool SymbolManager::RedeclarationError(std::string var_name)
{
    for(auto &table : tables)
    {
        if(table->LoopVarRedecl(var_name) == true)
        {
            return true;
        }
    }
    return tables.back()->VarRedecl(var_name);
}

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    //1
    SymbolTable *cur_table = new SymbolTable(symbol_manager.getLevel());
	symbol_manager.pushScope(cur_table);
    
    //2
    cur_table->addSymbol(p_program.getNameCString(), "program", cur_table->getLevel(), "void", p_program.getType());
    return_type_manager.function_in.push_back(p_program.getNameCString());
    symbol_manager.cur_root.push_back("program");
    
    //3
	p_program.visitChildNodes(*this);
	symbol_manager.cur_root.pop_back();
	return_type_manager.function_in.pop_back();
    // printf("\n\nhihi\n\n");
    //4
    if(opt_dump) 
    {
        dumpSymbol(cur_table);
    }

    //5
    symbol_manager.popScope();
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    //1
    SymbolTable *cur_table = symbol_manager.getTabelTop();

    //2

        /*redecl error*/
    if(symbol_manager.RedeclarationError(p_variable.getNameCString()))
    {
		m_error_printer.print(SymbolRedeclarationError(p_variable.getLocation(), p_variable.getNameCString()));
		assignment_manager.error_b = true;
		return;
	}
        /*demension error*/
    bool wrong_dimension = false;
    if(p_variable.DimensionError() == true)
    {
        m_error_printer.print(NonPositiveArrayDimensionError(p_variable.getLocation(), p_variable.getNameCString()));
        wrong_dimension = true;
    }

        /*normal*/
	if(symbol_manager.cur_root.back() == "function")
    {
		cur_table->addSymbol(p_variable.getNameCString(), "parameter", cur_table->getLevel(), p_variable.getTypeCString(), wrong_dimension, p_variable.getType());
		return;
	}
    else if(symbol_manager.cur_root.back()=="for")
    {
		cur_table->addSymbol(p_variable.getNameCString(), "loop_var", cur_table->getLevel(), p_variable.getTypeCString(), wrong_dimension, p_variable.getType());
		return;
	}
    else
    {
		cur_table->addSymbol(p_variable.getNameCString(), "variable", cur_table->getLevel(), p_variable.getTypeCString(), wrong_dimension, p_variable.getType());
	}
	
    //3
	symbol_manager.cur_root.push_back("variable");
	p_variable.visitChildNodes(*this);
	
    //5
    symbol_manager.cur_root.pop_back();
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_constant_value.type_ptr = p_constant_value.getPType();
    //2.
    if(symbol_manager.cur_root.back() == "variable")
    {
		SymbolTable *cur_table = symbol_manager.getTabelTop();
		cur_table->addConstantEntry(p_constant_value.getConstantValueCString());
	}

        /*non-integer idx error*/
    for(auto &root : symbol_manager.cur_root)
    {
		if(root == "variable_ref" && symbol_manager.cur_root.back() != "binary")
        {
			if(p_constant_value.getType() != "integer")
            {
				//printf("1234\n");
                if(constent_error == false){
                    m_error_printer.print(NonIntegerArrayIndexError(p_constant_value.getLocation()));
                    print_error = true;
                    read_error = true;
                    constent_error = true;
                    //printf("21234\n");
                    // symbol_manager.cur_root.pop_back();
                }
            }
			return;
		}
	}
    
    if(symbol_manager.cur_root.back() == "binary" || symbol_manager.cur_root.back() == "unary")
    {
		context_manager.expression_type.push_back(&*p_constant_value.getTypeSharedPtr());
	}
    if(symbol_manager.cur_root.back() == "function_invocation" && context_manager.parameter_error == false)
    {
		std::string func_type = static_cast<std::string>(p_constant_value.getTypeSharedPtr()->getPTypeCString());
        PType *para_type = symbol_manager.getParameterType(context_manager.function_name.back(), context_manager.parameter_num);
        if(func_type != static_cast<std::string>(para_type->getPTypeCString()))
        {
            if(func_type == "integer" && std::string(para_type->getPTypeCString()) == "real")
            {

            }
            else{
                //printf("123\n\nfunc_type %s   para_type %s\n", func_type.c_str(), para_type->getPTypeCString());
                m_error_printer.print(IncompatibleArgumentTypeError(p_constant_value.getLocation(), para_type, func_type));
                print_error = true;
                context_manager.parameter_error = true;
                return;
            }
        }
        context_manager.parameter_num = context_manager.parameter_num + 1;
	}
    if(symbol_manager.cur_root.back() == "assignment")
    {
		assignment_manager.assignment_type.push_back(&*p_constant_value.getTypeSharedPtr());
    }

        /*for start and end*/
	if(symbol_manager.cur_root.back() == "for")
    {
		for_manager.loop_end = stoi(p_constant_value.getConstantValueCString());
	}
	if(symbol_manager.cur_root.back() == "assignment")
    {
		if(symbol_manager.cur_root[symbol_manager.cur_root.size() - 2] == "for")
        {
            for_manager.loop_start = stoi(p_constant_value.getConstantValueCString());
        }
        
	}
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    
        /*redecl func error*/
    if(symbol_manager.RedeclarationError(p_function.getNameCString()) == true)
    {
        m_error_printer.print(SymbolRedeclarationError(p_function.getLocation(), p_function.getNameCString()));
    }
    else
    {
        SymbolTable *cur_function_program = symbol_manager.getTabelTop();
        cur_function_program->addSymbol(p_function.getNameCString(),"function", cur_function_program->getLevel(),p_function.getTypeCString(), p_function.getParameterString(),p_function.getType(),p_function.getParametersType());
    }

    //1.
    SymbolTable *cur_table = new SymbolTable(symbol_manager.getLevel(),p_function.getNameCString());
	symbol_manager.pushScope(cur_table);
	
    //2.
    return_type_manager.function_in.push_back(p_function.getNameCString());
	symbol_manager.cur_root.push_back("function");
	
    //3.
    p_function.visitChildNodes(*this);
	symbol_manager.cur_root.pop_back();
	return_type_manager.function_in.pop_back();
    
    //4.
    if(opt_dump)
    {
        dumpSymbol(cur_table);
    } 
	
    //5.
    symbol_manager.popScope();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    if(symbol_manager.cur_root.back() == "function")
    {
		symbol_manager.cur_root.push_back("compound");
		p_compound_statement.visitChildNodes(*this);
		symbol_manager.cur_root.pop_back();
	}
    else
    {
		//1.
        SymbolTable *cur_table = new SymbolTable(symbol_manager.getLevel());
		symbol_manager.pushScope(cur_table);
        symbol_manager.cur_root.push_back("compound");
		
        //3.
        p_compound_statement.visitChildNodes(*this);
		symbol_manager.cur_root.pop_back();
		
        //4.
        if(opt_dump)
        {
            dumpSymbol(cur_table);
        }

        //5.
        symbol_manager.popScope();
    }
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    //2.
    symbol_manager.cur_root.push_back("print");

    //3.
	p_print.visitChildNodes(*this);

    //4.
    print_error = false;

    //5.
	symbol_manager.cur_root.pop_back();
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    //2.
    symbol_manager.cur_root.push_back("binary");
	
    //3.
    p_bin_op.visitChildNodes(*this);
	
    //5.
    symbol_manager.cur_root.pop_back();
    
    //4.
    PType *type_right_oper = context_manager.expression_type.back();
	context_manager.expression_type.pop_back();
    std::string operand = p_bin_op.getOpCString();
	PType *type_left_oper = context_manager.expression_type.back();
	context_manager.expression_type.pop_back();

    if(static_cast<std::string>(type_right_oper->getPTypeCString()) == "error" || static_cast<std::string>(type_left_oper->getPTypeCString()) == "error")
    {
        return;
    }
    //printf("1\n")
    if(operand == "+" || operand == "-" || operand == "*" || operand == "/")
    {
        //printf("2\n");
        if(static_cast<std::string>(type_right_oper->getPTypeCString()) == "integer")
        {
            if(static_cast<std::string>(type_left_oper->getPTypeCString()) != "integer" && static_cast<std::string>(type_left_oper->getPTypeCString()) != "real")
            {
                m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.get_m_op(), type_left_oper, type_right_oper));
                context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
                if_error = true;
				while_error = true;
            }
            else
            {
                p_bin_op.type_ptr = type_left_oper;
                if(symbol_manager.cur_root.back()=="binary")
                {
                    context_manager.expression_type.push_back(type_left_oper);
                }
                //p_bin_op.setNodeType(type_left_oper->getPTypeCString());
            }
        }
        else if(static_cast<std::string>(type_right_oper->getPTypeCString()) == "real")
        {
            if(static_cast<std::string>(type_left_oper->getPTypeCString()) != "integer" && static_cast<std::string>(type_left_oper->getPTypeCString()) != "real")
            {
                m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.get_m_op(), type_left_oper, type_right_oper));
                context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
                if_error = true;
				while_error = true;
            }
            else
            {
                p_bin_op.type_ptr = type_right_oper;
                if(symbol_manager.cur_root.back() == "binary")
                {
					context_manager.expression_type.push_back(type_right_oper);
				}
                //p_bin_op.setNodeType(type_left_oper->getPTypeCString());
            }
        }
        else if(static_cast<std::string>(type_left_oper->getPTypeCString()) == "string" && operand == "+" && static_cast<std::string>(type_right_oper->getPTypeCString()) == "string")
        {
            p_bin_op.type_ptr = type_right_oper;
            if(symbol_manager.cur_root.back()=="binary")
            {
                context_manager.expression_type.push_back(type_right_oper);
            }
            //p_bin_op.setNodeType(type_left_oper->getPTypeCString());
        }
        else
        {
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.get_m_op(), type_left_oper, type_right_oper));
            context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
            if_error = true;
            while_error = true;
            
        }
    }
    else if(operand == "mod")
    {
        if(static_cast<std::string>(type_right_oper->getPTypeCString()) != "integer" || static_cast<std::string>(type_left_oper->getPTypeCString()) != "integer")
        {
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.get_m_op(), type_left_oper, type_right_oper));
            context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
            if_error = true;
            while_error = true;
        }
        else
        {
            p_bin_op.type_ptr = type_right_oper;
            if(symbol_manager.cur_root.back()=="binary")
            {
                context_manager.expression_type.push_back(type_right_oper);
            }
            //p_bin_op.setNodeType("integer");
        }
    }
    else if(operand == "<" || operand == "<=" || operand == ">" || operand == ">=" || operand == "=" || operand == "<>")
    {
        if((static_cast<std::string>(type_left_oper->getPTypeCString()) != "integer" && static_cast<std::string>(type_left_oper->getPTypeCString()) != "real") || (static_cast<std::string>(type_right_oper->getPTypeCString()) != "integer" && static_cast<std::string>(type_right_oper->getPTypeCString()) != "real"))
        {
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.get_m_op(), type_left_oper, type_right_oper));
            context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
            if_error=true;
            while_error=true;
        }
        else
        {
            p_bin_op.type_ptr=new PType(PType::PrimitiveTypeEnum::kBoolType);
            if(symbol_manager.cur_root.back() == "binary")
            {
                context_manager.expression_type.push_back(p_bin_op.type_ptr);
            }
            //p_bin_op.setNodeType("boolean");

        }
    }
    else if(operand == "and" || operand == "or")
    {
        if(static_cast<std::string>(type_left_oper->getPTypeCString()) != "boolean" || static_cast<std::string>(type_right_oper->getPTypeCString()) != "boolean")
        {
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.get_m_op(), type_left_oper, type_right_oper));
            context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
            if_error=true;
            while_error=true;
        }
        else
        {
            p_bin_op.type_ptr = type_right_oper;
            if(symbol_manager.cur_root.back()=="binary")
            {
                context_manager.expression_type.push_back(type_right_oper);
            }
            //p_bin_op.setNodeType("boolean");
        }
    }
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    //1.
    symbol_manager.cur_root.push_back("unary");

    //3.
	p_un_op.visitChildNodes(*this);
	
    //5.
    symbol_manager.cur_root.pop_back();

    //4.
    std::string operand = p_un_op.getOpCString();
    PType *operand_type = context_manager.expression_type.back();
	context_manager.expression_type.pop_back();
    //printf("\n\n%s\n\n", operand_type->getPTypeCString());
    if(static_cast<std::string>(operand_type->getPTypeCString())=="error")
    {
        return;
    }

    if(operand == "not")
    {
        //printf("\n\njfsogjsjdfgo\n\n");
        if(static_cast<std::string>(operand_type->getPTypeCString()) != "boolean")
        {
            m_error_printer.print(InvalidUnaryOperandError(p_un_op.getLocation(), p_un_op.get_m_op(), operand_type));
            if_error = true;
            while_error = true;
            if(symbol_manager.cur_root.back() == "assignment"){ 
                assignment_manager.assignment_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
            }
        }
        else
        {
            p_un_op.type_ptr = operand_type;
			if(symbol_manager.cur_root.back() == "unary")
            {
				context_manager.expression_type.push_back(operand_type);
			}
            if(symbol_manager.cur_root.back() == "assignment"){ 
                assignment_manager.assignment_type.push_back(operand_type);
            }
        }
    }
    else if(operand == "-" || operand == "neg")
    {
        
        // if(symbol_manager.cur_root.back() == "function_invocation" && context_manager.parameter_error == false)
        // {
        //     std::string func_type = static_cast<std::string>(p_constant_value.getTypeSharedPtr()->getPTypeCString());
        //     PType *para_type = symbol_manager.getParameterType(context_manager.function_name.back(), context_manager.parameter_num);
        //     if(func_type != static_cast<std::string>(para_type->getPTypeCString()))
        //     {
        //         m_error_printer.print(IncompatibleArgumentTypeError(p_constant_value.getLocation(), para_type, func_type));
        //         print_error = true;
        //         context_manager.parameter_error = true;
        //         return;
        //     }
        //     context_manager.parameter_num = context_manager.parameter_num + 1;
        // }
        //printf("\n\nafildfjialfja\n\n");
        if(static_cast<std::string>(operand_type->getPTypeCString()) != "real" && static_cast<std::string>(operand_type->getPTypeCString()) != "integer")
        {
            m_error_printer.print(InvalidUnaryOperandError(p_un_op.getLocation(), p_un_op.get_m_op(), operand_type));
            if_error = true;
            while_error = true;
            if(symbol_manager.cur_root.back() == "function_invocation"){
                context_manager.parameter_error = true;
                print_error = true;
            }
            if(symbol_manager.cur_root.back() == "assignment"){ 
                assignment_manager.assignment_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
            }
        }
        else
        {
            p_un_op.type_ptr = operand_type;
			if(symbol_manager.cur_root.back() == "unary")
            {
				context_manager.expression_type.push_back(operand_type);
			}
            if(symbol_manager.cur_root.back() == "assignment"){ 
                assignment_manager.assignment_type.push_back(operand_type);
            }
        }
    }
    
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
        /*undeclared error*/
    if(symbol_manager.UndeclaredError(p_func_invocation.getNameCString()) == false)
    {
        m_error_printer.print(UndeclaredSymbolError(p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
		return;
    }
        /*non-func error*/
    if(symbol_manager.NonFunctionSymbolError(p_func_invocation.getNameCString()) == false)
    {
        m_error_printer.print(NonFunctionSymbolError(p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
		return;
    }

        /*#para wrong error*/
    if(symbol_manager.ArgumentNumberMismatchError(p_func_invocation.getNameCString(), p_func_invocation.getParaSize()) == true)
    {
        m_error_printer.print(ArgumentNumberMismatchError(p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
        return;
    }

    p_func_invocation.type_ptr = symbol_manager.getFunctionType(p_func_invocation.getNameCString());
    if(symbol_manager.cur_root.back() == "function_invocation" && context_manager.parameter_error == false)
    {
        std::string func_type = static_cast<std::string>(symbol_manager.getFunctionType(p_func_invocation.getNameCString())->getPTypeCString());
        PType *para_type = symbol_manager.getParameterType(context_manager.function_name.back(), context_manager.parameter_num);
        if(func_type != static_cast<std::string>(para_type->getPTypeCString()))
        {
            if(func_type == "integer" && std::string(para_type->getPTypeCString()) == "real")
            {

            }
            else
            {
                //printf("\n\nfunc_type %s   para_type %s\n", func_type, para_type->getPTypeCString());
                m_error_printer.print(IncompatibleArgumentTypeError(p_func_invocation.getLocation(), para_type, func_type));
                context_manager.parameter_error = true;
                return;
            }
            
        }
        context_manager.parameter_num = context_manager.parameter_num + 1;
    }

    if(symbol_manager.cur_root.back() == "assignment")
    {
		assignment_manager.assignment_type.push_back(symbol_manager.getType(p_func_invocation.getNameCString()));
	}
    if(symbol_manager.cur_root.back() == "binary" || symbol_manager.cur_root.back() == "unary")
    {
		context_manager.expression_type.push_back(symbol_manager.getType(p_func_invocation.getNameCString()));
	}

	symbol_manager.cur_root.push_back("function_invocation");
    context_manager.function_name.push_back(p_func_invocation.getNameCString());
	p_func_invocation.visitChildNodes(*this);
	symbol_manager.cur_root.pop_back();
	context_manager.function_name.pop_back();
	context_manager.parameter_num = 0;
	context_manager.parameter_error = false;

        /*print non-scalar error*/
    if(symbol_manager.cur_root.back() == "print")
    {
		if(print_error == false)
        {
            if(p_func_invocation.type_ptr->getType() == "void")
            {
                m_error_printer.print(PrintOutNonScalarTypeError(p_func_invocation.getLocation()));
                print_error = true;
            }
        }
	}
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    constent_error = false;
        /*undeclared error*/
    if(symbol_manager.UndeclaredError(p_variable_ref.getNameCString()) == false)
    {
        m_error_printer.print(UndeclaredSymbolError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
        assignment_manager.error_b = true;
		return;
    }
        /*non-variable error*/
    if(symbol_manager.NonVariableError(p_variable_ref.getNameCString()) == false)
    {
        m_error_printer.print(NonVariableSymbolError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
        assignment_manager.error_b = true;
		return;
    }

    if(symbol_manager.WrongDeclOrNot(p_variable_ref.getNameCString()) == true){ return; }
    
    //     /*over array subscript error*/
    // if(symbol_manager.OverArraySubscriptError(p_variable_ref.getNameCString(), p_variable_ref.getSizeOfDimension()) == true)
    // {
    //     m_error_printer.print(OverArraySubscriptError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
    //     context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
    //     assignment_manager.error_b = true;
    //     print_error = true;
	// 	read_error = true;
    //     return;
    // }

    //2.
    p_variable_ref.type_ptr=symbol_manager.getType(p_variable_ref.getNameCString());
	symbol_manager.cur_root.push_back("variable_ref");

    //3. 5.
    p_variable_ref.visitChildNodes(*this);
	symbol_manager.cur_root.pop_back();
    
        /*over array subscript error*/
    if(symbol_manager.OverArraySubscriptError(p_variable_ref.getNameCString(), p_variable_ref.getSizeOfDimension()) == true)
    {
        //printf("5678\n");
        if(constent_error == false){
            //printf("25678\n");
            m_error_printer.print(OverArraySubscriptError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
            context_manager.expression_type.push_back(new PType(PType::PrimitiveTypeEnum::kErrorType));
            assignment_manager.error_b = true;
            print_error = true;
            read_error = true;
        }
        return;
    }

    //4.
    if(symbol_manager.cur_root.back()=="binary" || symbol_manager.cur_root.back() == "unary")
    {
		PType *cur_type = new PType(symbol_manager.getType(p_variable_ref.getNameCString())->getPrimitiveType());
		std::vector<uint64_t> cur_dim;
        int size_type_dimension = symbol_manager.getType(p_variable_ref.getNameCString())->getDimensions().size();
		for(int i = 0; i < size_type_dimension; i++)
        {
			if(i >= p_variable_ref.getSizeOfDimension())
            {
				cur_dim.push_back(symbol_manager.getType(p_variable_ref.getNameCString())->getDimensions()[i]);
			}
		}
		cur_type->setDimensions(cur_dim);
		context_manager.expression_type.push_back(cur_type);
	}
    if(symbol_manager.cur_root.back() == "function_invocation" && context_manager.parameter_error == false)
    {
		std::string func_type = static_cast<std::string>(symbol_manager.getType(p_variable_ref.getNameCString())->getPTypeCString());
        PType *para_type = symbol_manager.getParameterType(context_manager.function_name.back(), context_manager.parameter_num);
        if(func_type != static_cast<std::string>(para_type->getPTypeCString()))
        {
            if(func_type == "integer" && std::string(para_type->getPTypeCString()) == "real")
            {

            }
            else
            {
                //printf("\n\nfunc_type %s   para_type %s\n", func_type.c_str(), para_type->getPTypeCString());
                m_error_printer.print(IncompatibleArgumentTypeError(p_variable_ref.getLocation(), para_type, func_type));
                print_error = true;
                read_error=true;
                context_manager.parameter_error = true;
                return;
            }
            
        }
        context_manager.parameter_num = context_manager.parameter_num + 1;
	}
    if(symbol_manager.cur_root.back() == "print")
    {
		if(print_error == false)
        {
            int type_dim = p_variable_ref.type_ptr->getDimensionsSize();
            int actual_dim = p_variable_ref.getSizeOfDimension();
            if(type_dim > actual_dim)
            {
                m_error_printer.print(PrintOutNonScalarTypeError(p_variable_ref.getLocation()));
                print_error = true;
            }
        }
	}
    if(symbol_manager.cur_root.back() == "read")
    {
        if(read_error == false)
        {
            int type_dim = p_variable_ref.type_ptr->getDimensionsSize();
            int actual_dim = p_variable_ref.getSizeOfDimension();
            bool loop_var = symbol_manager.LoopVarOrNot(p_variable_ref.getNameCString());
            std::string know_kind = symbol_manager.getKind(p_variable_ref.getNameCString());
            if((p_variable_ref.type_ptr->getType()=="void") || (type_dim > actual_dim))
            {
                m_error_printer.print(ReadToNonScalarTypeError(p_variable_ref.getLocation()));
                read_error = true;
            }
            else if((loop_var == true) || (know_kind == "constant"))
            {
                m_error_printer.print(ReadToConstantOrLoopVarError(p_variable_ref.getLocation()));
                read_error = true;
            }
        }
    }
    if(symbol_manager.cur_root.back() == "assignment")
    {
		PType *cur_type = new PType(symbol_manager.getType(p_variable_ref.getNameCString())->getPrimitiveType());
		std::vector<uint64_t> cur_dim;
		for(int i = 0; i < symbol_manager.getType(p_variable_ref.getNameCString())->getDimensions().size(); i++)
        {
			if(i >= p_variable_ref.getSizeOfDimension())
            {
				cur_dim.push_back(symbol_manager.getType(p_variable_ref.getNameCString())->getDimensions()[i]);
			}
		}
		cur_type->setDimensions(cur_dim);
		assignment_manager.assignment_type.push_back(cur_type);
	}
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    //1.
    symbol_manager.cur_root.push_back("assignment");
    //printf("\n\ntt\n\n");
    //3.
	p_assignment.visitChildNodes(*this);

    //5.
	symbol_manager.cur_root.pop_back();

    //4.
    VariableReferenceNode *p_var_ref = p_assignment.getVarNode();
    ExpressionNode *p_expr_node = p_assignment.getExprNode();
    std::string var_name = p_var_ref->getNameCString();

    if(assignment_manager.error_b == false)
    {
        if(p_assignment.getVarDim() > 0)
        {
            m_error_printer.print(AssignWithArrayTypeError(p_var_ref->getLocation()));
            assignment_manager.error_b = true;
        }
        else if(symbol_manager.getKind(var_name) == "constant")
        {
            m_error_printer.print(AssignToConstantError(p_var_ref->getLocation(), p_var_ref->getNameCString()));
            assignment_manager.error_b = true;
        }
        else if(symbol_manager.cur_root.back() != "for" && symbol_manager.getKind(var_name) == "loop_var")
        {
            m_error_printer.print(AssignToLoopVarError(p_var_ref->getLocation()));
            assignment_manager.error_b = true;
        }
        else if(p_assignment.getExprDim() > 0)
        {
            m_error_printer.print(AssignWithArrayTypeError(p_expr_node->getLocation()));
            assignment_manager.error_b = true;
        }
    }
    if(assignment_manager.error_b == true)
    {
        assignment_manager.error_b = false;
		assignment_manager.assignment_type.clear();
		return;
    }

    PType *rhs_type = assignment_manager.assignment_type.back();
    assignment_manager.assignment_type.pop_back();
    //printf("\n\nr %ld\n", assignment_manager.assignment_type.size());
    PType *lhs_type = assignment_manager.assignment_type.back();
    assignment_manager.assignment_type.pop_back();
    //printf("r %ld\n", assignment_manager.assignment_type.size());
    //cout<<lhs_type<<;
    
    std::string rhs_type_string = static_cast<std::string>(rhs_type->getPTypeCString());
    //printf("%s  %s\n\n", var_name.c_str(), rhs_type->getPTypeCString());
    //printf("\n\n%s\n\n", lhs_type->getPTypeCString());
    std::string lhs_type_string = static_cast<std::string>(lhs_type->getPTypeCString());
    if(rhs_type_string == "error" || lhs_type_string == "error")
    {
        return;
    }
    else if(rhs_type_string != lhs_type_string && !(rhs_type_string == "error" || lhs_type_string == "error"))
    {
        if(!(rhs_type_string == "integer" && lhs_type_string == "real"))
        {
            m_error_printer.print(IncompatibleAssignmentError(p_assignment.getLocation(), lhs_type, rhs_type));
        }
    }
    
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    //1
    symbol_manager.cur_root.push_back("read");
	
    //3.
    p_read.visitChildNodes(*this);
	
    //4.
    read_error = false;

    //5.
    symbol_manager.cur_root.pop_back();
}

void SemanticAnalyzer::visit(IfNode &p_if) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    //1. 
    symbol_manager.cur_root.push_back("if");
	
    //3.
    p_if.visitChildNodes(*this);
	
    //5.
    symbol_manager.cur_root.pop_back();

    //4.
	if(static_cast<std::string>(p_if.getConditionType()) != "boolean")
    {
		if(if_error == false)
        {
            m_error_printer.print(NonBooleanConditionError(p_if.getCondition()->getLocation()));
        }
	}
	if_error = false;
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    //1.
    symbol_manager.cur_root.push_back("while");

    //3.
	p_while.visitChildNodes(*this);

    //5.
	symbol_manager.cur_root.pop_back();


    //4.
    if(static_cast<std::string>(p_while.getConditionType()) != "boolean")
    {
		if(while_error == false)
        {
            m_error_printer.print(NonBooleanConditionError(p_while.getCondition()->getLocation()));
        }
	}
	while_error = false;
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    //1. 
    SymbolTable *cur_table = new SymbolTable(symbol_manager.getLevel());
	symbol_manager.pushScope(cur_table);
	symbol_manager.cur_root.push_back("for");
	
    //3.
    p_for.visitChildNodes(*this);

    //5.
	symbol_manager.cur_root.pop_back();

    //4.
    if(for_manager.loop_error() == true)
    {
        m_error_printer.print(NonIncrementalLoopVariableError(p_for.getLocation()));
    }

    if(opt_dump) 
    {
        dumpSymbol(cur_table);
    }

    //5.
    symbol_manager.popScope();
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    //1.
    symbol_manager.cur_root.push_back("return");

    //3.
	p_return.visitChildNodes(*this);

    //5.
	symbol_manager.cur_root.pop_back();

    //4.
    std::string enter_function_name = return_type_manager.function_in.back();
    if(return_type_manager.function_in.size() == 1 || symbol_manager.getFunctionType(enter_function_name)->getType() == "void")
    {
        m_error_printer.print(ReturnFromVoidError(p_return.getLocation()));
        return;
    }

    ExpressionNode *ret_expr = p_return.get_m_ret_val();
    std::string expression_type = return_type_manager.getReturnTypeString(ret_expr->getSizeOfDimension(), ret_expr->type_ptr);
    PType *func_type = symbol_manager.getFunctionType(enter_function_name);

    // printf("enter_function_name %s  expression_type %s   func_type %s\n\n", enter_function_name.c_str(), ret_expr->type_ptr->getPTypeCString(), func_type->getPTypeCString());
    // if(strcmp(ret_expr->type_ptr->getPTypeCString(), func_type->getPTypeCString())!=0)
    // {
    //     m_error_printer.print(IncompatibleReturnTypeError(ret_expr->getLocation(), func_type, expression_type));
    // }
    
    if(expression_type != static_cast<std::string>(func_type->getPTypeCString()) && expression_type != "error")
    {
		if(static_cast<std::string>(func_type->getPTypeCString()) == "real" && expression_type == "integer")
        {

        }
        else
        {
            m_error_printer.print(IncompatibleReturnTypeError(ret_expr->getLocation(), func_type, expression_type));
        }
        
	}
}
