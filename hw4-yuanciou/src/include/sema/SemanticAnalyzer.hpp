#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "sema/ErrorPrinter.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include "AST/PType.hpp"
#include <vector>
#include <string>
#include <stack>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

class SymbolEntry {
private:
    std::string name;
    std::string kind;
    int level;
    std::string type;
	  PType *type_ptr;
    std::string attribute;
	  bool declaration_error = false;
	  int dimension_num;
	  std::vector<PType*> parameters_type;
    int integer_value;
    float real_value;
    bool bool_value;
    std::string string_value;
public:
	SymbolEntry(std::string p_name, std::string p_kind, int p_level, std::string p_type, PType *p_type_ptr)
            : name(p_name), kind(p_kind), level(p_level), type(p_type), type_ptr(p_type_ptr)
			      { dimension_num = count(type.begin(), type.end(), '['); }

	SymbolEntry(std::string p_name, std::string p_kind, int p_level, std::string p_type, std::string p_attribute, PType *p_type_ptr, std::vector<PType*> p_parameters_type)
            : name(p_name), kind(p_kind), level(p_level), type(p_type), attribute(p_attribute), type_ptr(p_type_ptr), parameters_type(p_parameters_type)
				    { dimension_num = count(type.begin(), type.end(), '['); }
	
	SymbolEntry(std::string p_name, std::string p_kind, int p_level, std::string p_type, bool p_declaration_error, PType *p_type_ptr)
            : name(p_name), kind(p_kind), level(p_level), type(p_type), declaration_error(p_declaration_error), type_ptr(p_type_ptr)
			      { dimension_num = count(type.begin(), type.end(), '['); }

	void PrintEntry()
  {
		std::string string_level = (level == 0) ? (std::to_string(level) + "(global)") : (std::to_string(level) + "(local)");
    printf("%-33s%-11s%-11s%-17s%-11s\n", name.c_str(), kind.c_str(), string_level.c_str(), type.c_str(), attribute.c_str());
	}

  void addConstant(std::string value)
  {
    kind = "constant";
    attribute = value;
    if(type == "integer")
    {
      integer_value = stoi(attribute, NULL, 10);
    }
    else if(type == "real")
    {
      real_value = stof(attribute);
    }
    else if(type == "string")
    {
      string_value = attribute;
    }
    else if(type == "boolean")
    {
      bool_value = (attribute == "true") ? true : false;
    }
  }

	PType *getType(std::string cur_name) { return ((cur_name == name) ? type_ptr : nullptr); }

  std::pair<std::vector<PType*>, bool> getParameterType(std::string cur_name)
  {
    std::vector<PType*> parameter_type;
    return ((cur_name == name && kind == "function") ? make_pair(parameters_type, true) : make_pair(parameter_type, false));
  }

	std::string getKind(std::string cur_name) { return ((cur_name == name) ? kind : ""); }

	bool VarRedecl(std::string cur_name) { return ((cur_name == name) ? true : false); }
  bool LoopVarRedecl(std::string cur_name) { return ((cur_name == name && kind == "loop_var") ? true : false); }
  bool VarKind(std::string cur_name) { return ((cur_name == name && (kind=="parameter" || kind=="variable" || kind=="loop_var" || kind=="constant")) ? true : false); }
	bool NonFunctionSymbol(std::string cur_name) { return ((cur_name == name && kind == "function") ? true : false); }
	bool OverArraySubscript(std::string cur_name, int cur_size) { return (((cur_name == name) && (cur_size > dimension_num)) ? true : false); }
	bool WrongDeclOrNot(std::string cur_name) { return ((cur_name == name) ? declaration_error : false); }
	bool ParameterOver(std::string cur_name, int cur_size) { return ((cur_name == name && cur_size!=parameters_type.size()) ? true : false); }
};

class SymbolTable {
public:
	SymbolTable(int p_level)
            : level(p_level){}
	SymbolTable(int p_level, string p_function_name)
            : level(p_level), function_name(p_function_name) {}

  void addSymbol(std::string p_name, std::string p_kind, int p_level, std::string p_type, PType *p_type_ptr)
                {
									SymbolEntry symbol_entry(p_name, p_kind, p_level, p_type, p_type_ptr);
								  entries.push_back(symbol_entry);
								}
	void addSymbol(std::string p_name, std::string p_kind, int p_level, std::string p_type, std::string p_attribute, PType *p_type_ptr, std::vector<PType*> p_parameters_type)
                {
									SymbolEntry symbol_entry(p_name, p_kind, p_level, p_type, p_attribute, p_type_ptr, p_parameters_type);
								  entries.push_back(symbol_entry);
								}
	void addSymbol(std::string p_name, std::string p_kind, int p_level, std::string p_type, bool p_declaration_error, PType *p_type_ptr)
                {
									SymbolEntry symbol_entry(p_name, p_kind, p_level, p_type, p_declaration_error, p_type_ptr);
								  entries.push_back(symbol_entry);
								}

  // other methods
	void PrintTable()
  {
		for(auto &entry : entries){
			entry.PrintEntry();
		}
	}

	void addConstantEntry(std::string constant){ entries.back().addConstant(constant); }
  int getLevel() { return level; }

	PType *getType(string cur_name) 
  {
		for(auto &entry : entries)
    {
			if(entry.getType(cur_name) != nullptr)
      {
				return entry.getType(cur_name);
			}
		}
		return nullptr;
	}

	std::pair<std::vector<PType*>, bool> getParameterType(string cur_name)
  {
		std::vector<PType*> new_type;
		for(auto &entry : entries)
    {
			if(entry.getParameterType(cur_name).second == true)
      {
				return entry.getParameterType(cur_name);
			}
		}
		return make_pair(new_type, false);
	}

	std::string getKind(std::string cur_name) 
  {
		for(auto &entry : entries)
    {
			if(entry.getKind(cur_name) != "")
      {
				return entry.getKind(cur_name);
			}
		}
		return "";
	}

	bool VarRedecl(std::string cur_name)
  {
		for(auto &entry : entries)
    {
			if(entry.VarRedecl(cur_name) == true)
      {
				return true;
			}
		}
		return false;
	}

  bool LoopVarRedecl(std::string cur_name)
  {
		for(auto &entry : entries)
    {
			if(entry.LoopVarRedecl(cur_name) == true)
      {
				return true;
			}
		}
		return false;
	}

  bool VarKind(std::string cur_name)
  {
		for(auto &entry : entries)
    {
			if(entry.VarKind(cur_name) == true)
      {
				return true;
			}
		}
		return false;
	}
	
  bool NonFunctionSymbol(std::string cur_name)
  {
		for(auto &entry : entries)
    {
			if(entry.NonFunctionSymbol(cur_name) == true)
      {
				return true;
			}
		}
		return false;
	}
	
  bool OverArraySubscript(std::string cur_name, int cur_dim)
  {
		for(auto &entry : entries)
    {
			if(entry.OverArraySubscript(cur_name, cur_dim))
      {
				return true;
			}
		}
		return false;
	}
	
	bool WrongDeclOrNot(std::string cur_name)
  {
		for(auto &entry : entries)
    {
			if(entry.WrongDeclOrNot(cur_name))
      {
				return true;
			}
		}
		return false;
	}

	bool ParameterOver(std::string cur_name, int cur_size)
  {
		for(auto &entry : entries)
    {
			if(entry.ParameterOver(cur_name, cur_size))
      {
				return true;
			}
		}
		return false;
	}
    
private:
  std::vector<SymbolEntry> entries;
	int level;
	std::string function_name;
};

class SymbolManager {
public:
  void pushScope(SymbolTable *new_scope) { level++; tables.push_back(new_scope); }
  void popScope() { level--; tables.pop_back(); }
  // other methods
  std::vector<std::string> cur_root;
	int getLevel() { return level; }
	SymbolTable *getTabelTop() { return tables.back(); }

	PType *getType(std::string cur_name) 
  {
		for(int i = tables.size() - 1; i >= 0; i--)
    {
			if(tables[i]->getType(cur_name) != nullptr)
      {
				return tables[i]->getType(cur_name);
			}
		}
		return nullptr;
	}

  string getKind(std::string cur_name) 
  {
		for(int i = tables.size() - 1; i >= 0; i--)
    {
			if(tables[i]->getKind(cur_name) != "")
      {
				return tables[i]->getKind(cur_name);
			}
		}
		return "";
	}

	PType *getFunctionType(std::string cur_name) 
  {
		for(int i = tables.size() - 1; i >= 0; i--)
    {
			if(tables[i]->getType(cur_name) != nullptr && NonFunctionSymbolError(cur_name) == true)
      {
				return tables[i]->getType(cur_name);
			}
		}
		return nullptr;
	}
	
	bool RedeclarationError(std::string cur_name);

	bool UndeclaredError(std::string cur_name)
  {
		for(auto &table : tables)
    {
			if(table->VarRedecl(cur_name) == true)
      {
				return true;
			}
		}
		return false;
	}

	bool NonVariableError(std::string cur_name){
		for(auto &table : tables)
    {
			if(table->VarKind(cur_name) == true)
      {
				return true;
			}
		}
		return false;
	}

	bool NonFunctionSymbolError(std::string cur_name)
  {
		for(auto &table : tables)
    {
			if(table->NonFunctionSymbol(cur_name))
      {
				return true;
			}
		}
		return false;
	}

  bool OverArraySubscriptError(std::string cur_name, int cur_dim)
  {
		for(auto &table : tables)
    {
			if(table->OverArraySubscript(cur_name, cur_dim))
      {
				return true;
			}
		}
		return false;
	}

	bool WrongDeclOrNot(std::string cur_name){
		for(auto &table : tables)
    {
			if(table->WrongDeclOrNot(cur_name))
      {
				return true;
			}
		}
		return false;
	}

	bool ArgumentNumberMismatchError(std::string cur_name, int cur_size)
  {
		for(auto &table:tables)
    {
			if(table->ParameterOver(cur_name, cur_size))
      {
				return true;
			}
		}
		return false;
	}

	PType *getParameterType(std::string func_name, int parameter_idx)
  {
		for(auto &table:tables)
    {
			if(table->getParameterType(func_name).second == true)
      {
				return table->getParameterType(func_name).first[parameter_idx];
			}
		}
	}

	bool LoopVarOrNot(std::string cur_name)
  {
		for(auto &table:tables)
    {
			if(table->LoopVarRedecl(cur_name))
      {
				return true;
			}
		}
		return false;
	}
private:
  vector<SymbolTable *> tables;
  int level = 0;
};

class ContextManager{
public:
  int parameter_num = 0;
	bool parameter_error = false;
	std::vector<PType *> expression_type;
	std::vector<std::string> function_name;
};

class ReturnTypeManager{
public:
	std::vector<std::string> function_in;

	std::string getReturnTypeString(int dimension_number, PType *type)
  	{
		string return_type = type->getType();

		if(dimension_number < type->getDimensionsSize())
		{
			return_type = return_type + " ";
		}

		for(int i = dimension_number; i < type->getDimensionsSize(); i++)
    	{
			return_type = return_type + "[";
			return_type = return_type + to_string(type->getDimensions()[i]);
			return_type = return_type + "]";
		}
		return return_type;
	}
};

class ForManager{
public:
	int loop_start;
	int loop_end;
  bool loop_error() { return ((loop_start > loop_end) ? true : false); }
};

class AssignmentManager{
public:
	bool error_b = false;
	std::vector<PType *> assignment_type;
};

class SemanticAnalyzer final : public AstNodeVisitor {
  private:
    ErrorPrinter m_error_printer{stderr};
    // TODO: something like symbol manager (manage symbol tables)
    //       context manager, return type manager
    SymbolManager symbol_manager;
    ContextManager context_manager;
    ReturnTypeManager return_type_manager;
    ForManager for_manager;
    AssignmentManager assignment_manager;
    bool print_error = false;
    bool read_error = false;
    bool if_error = false;
    bool while_error = false;
	bool constent_error = false;

  public:
    ~SemanticAnalyzer() = default;
    SemanticAnalyzer() = default;

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
