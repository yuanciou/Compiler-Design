# hw3 report

|      |                 |
| ---: | :-------------- |
| Name | 邱振源        |
|   ID | 111550100 |

## How much time did you spend on this project

> About 16-18 hours.

## Project overview

### Changes in ```scanner.l```
I use the provided ```scanner.l```, but in the following part of the code, I pass the values to ```yylval``` using ```strtol()```, ```atof()```, and ```strndup()```.
```
/* Integer (decimal/octal) */
{integer} { ...; yylval.integer = strtol(yytext, NULL, 10); ... }
0[0-7]+   { ...; yylval.integer = strtol(yytext, NULL, 8); ... }

/* Floating-Point */
{float} { ...; yylval.real = atof(yytext); ... }

/* Scientific Notation [Ee][+-]?[0-9]+ */
(...) { ...; yylval.real = atof(yytext); ... }

/* String */
... {
    ...
    listLiteral("string", string_literal);
    yylval.identifier = strndup(string_literal, MAX_ID_LENG);
    ...
}
```

### Changes in ```parser.y```
In the ```%code requires{}``` section, I have included the header file ```ast.hpp```, as well as the libraries for ```string``` and ```vector```. Additionally, I have defined classes for different node types in the file. 
Furthermore, to better define the information for different variables, I have defined a type called *Variable*, so that declaring types and storing data below can be more convenient.
```
%code requires {
    #include "AST/ast.hpp"
    #include <vector>
    #include <string>
    class AstNode;
    class CompoundStatementNode;
    class DeclNode;
    class VariableNode;
    class ConstantValueNode;
    class ExpressionNode;
    class FunctionNode;
    class FunctionInvocationNode;
    class ProgramNode;
    class VariableReferenceNode;
    class AssignmentNode;
    class ReadNode;
    class PrintNode;
    class IfNode;
    class WhileNode;
    class ForNode;
    class ReturnNode;

    struct Variable {
        std::string type, val;
        size_t line, column;

        Variable(std::string type, std::string val) : type(type), val(val) {}
        Variable(std::string val, size_t line, size_t column) : type("ID"), val(val), line(line), column(column) {}
    };
}
```
In the ``%code union{}``` section, I define the types that yylval and the non-terminals under the parser can use.
```
%union {
    /* basic semantic value */
    char *identifier;
    int integer;
    double real;
    Variable *variable;
    CompoundStatementNode *compound_stmt_ptr;
    AstNode *node;
    std::vector<AstNode *> *node_list;
    DeclNode *decl_node;
    VariableNode *var_node;
    ConstantValueNode *const_val_node;
    ExpressionNode *expression_node;
    std::vector<ExpressionNode *> *experssion_list;
    std::vector<DeclNode *> *decl_list;
    std::vector<Variable *> *variable_list;
    ProgramNode *program_node;
    FunctionNode *function_node;
    std::vector<FunctionNode *> *function_list;
    FunctionInvocationNode *function_invo_node;
    VariableReferenceNode *var_re_node;
    AssignmentNode *assign_node;
    ReadNode *read_node;
    PrintNode *print_node;
    IfNode *if_node;
    WhileNode *while_node;
    ForNode *for_node;
    ReturnNode *return_node;
};
```
In the ```%type``` section, I assign types to the non-terminals respectively.
```
%type <identifier> ProgramName ID Type ScalarType IntegerAndReal ArrType ArrDecl STRING_LITERAL FunctionName ReturnType
%type <compound_stmt_ptr> CompoundStatement ElseOrNot
%type <decl_node> Declaration FormalArg
%type <decl_list> Declarations DeclarationList FormalArgs FormalArgList
%type <variable> LiteralConstant StringAndBoolean
%type <variable_list> IdList
%type <integer> NegOrNot INT_LITERAL
%type <real> REAL_LITERAL
%type <program_node> Program
%type <function_list> Functions FunctionList
%type <function_node> FunctionDeclaration FunctionDefinition Function
%type <node> ProgramUnit Statement Simple
%type <node_list> Statements StatementList
%type <experssion_list> ExpressionList Expressions ArrRefList ArrRefs
%type <expression_node> Expression 
%type <function_invo_node> FunctionInvocation FunctionCall
%type <var_re_node> VariableReference
%type <if_node> Condition
%type <while_node> While
%type <for_node> For
%type <return_node> Return
```
In the part of ```%token```, I specify the precedence of the operators.
```
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS
```
Finally, in the grammar section, I construct the AST using syntax like ```$$ = $1```.

### Construct AST & how visitor pattern works

Through *Abstract Syntax Definitions*, which is provided and comments in each file, we can define the parameters and values to be stored in the class of each different node in the ```.hpp file```. After that I, include the below header file to use visitor pattern.
```
#include "visitor/AstNodeVisitor.hpp"
```
Then, add the following member functions
```
void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
void visitChildNodes(AstNodeVisitor &p_visitor) override;
```
Finally, add the below code to the ```.cpp file```.
```
value->accept(p_visitor);
```
Take ReadNode for example:
*read.hpp*
```
#ifndef __AST_READ_NODE_H
#define __AST_READ_NODE_H

#include "AST/ast.hpp"
#include "AST/VariableReference.hpp"
#include "visitor/AstNodeVisitor.hpp"

class ReadNode : public AstNode {
  public:
    ReadNode(const uint32_t line, const uint32_t col,
             /* TODO: variable reference */
             VariableReferenceNode *const p_variable_re);
    ~ReadNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: variable reference
    VariableReferenceNode *m_variable_re;
};

#endif
```
*read.cpp*
```
#include "AST/read.hpp"

// TODO
ReadNode::ReadNode(const uint32_t line, const uint32_t col,
                   VariableReferenceNode *const p_variable_re)
    : AstNode{line, col}, m_variable_re(p_variable_re) {}

// TODO: You may use code snippets in AstDumper.cpp
void ReadNode::print() {}

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    m_variable_re->accept(p_visitor);
}
```
Sometimes, we also need to add member functions in the header file and in ```AstDumper.cpp``` so that we can obtain the information we need when dumping. Take *FunctionInvocationNode* for example:
```
/* FunctionInvocation.hpp */
// add the member function
const char *getNameCString() { return m_name.c_str(); };
```
```
/* AstDumper.cpp */
void AstDumper::visit(FunctionInvocationNode &p_func_invocation) {
    outputIndentationSpace(m_indentation);

    // TODO: function name
    std::printf("function invocation <line: %u, col: %u> %s\n",
                p_func_invocation.getLocation().line,
                p_func_invocation.getLocation().col,
                p_func_invocation.getNameCString());

    incrementIndentation();
    p_func_invocation.visitChildNodes(*this);
    decrementIndentation();
}
```
After completing each node, we have constructed the AST. At the same time, the ```accept()``` function allows the visitor to visit the node itself, while ```visitChildNodes()``` enables the visitor to traverse the AST smoothly to complete the dump. This way, the visitor pattern is applied throughout the AST.

## What is the hardest you think in this project

1. I found the most challenging part of this assignment to be grasping object-oriented code. Due to my limited familiarity with object-oriented concepts, understanding the relationships between classes and becoming acquainted with the syntax took me a considerable amount of time.
2. Additionally, understanding the workings of the visitor pattern also proved to be time-consuming. When completing the assignment, I often found myself mentally simulating the appearance of the AST multiple times and how the visitor pattern would operate within it.

## Feedback to T.A.s

Although this assignment shares similarities with the previous one, for those unfamiliar with object-oriented programming, it would be helpful to clearly outline the specific Node types or files needed for each test case. This way, as one progresses through the test cases step by step, they can have a clearer understanding of what needs to be completed first. Thank you to the teaching assistant for the thoughtful assignment design.
