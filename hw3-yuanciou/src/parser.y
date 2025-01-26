%{
#include <string>
#include <vector>

#include "AST/AstDumper.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern uint32_t line_num;   /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

// This guarantees that headers do not conflict when included together.
%define api.token.prefix {TOK_}

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

    /* For yylval */
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

    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
    /* TODO: specify the precedence of the following operators */
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%

ProgramUnit:
    Program { $$ = $1; }
    |
    Function { $$ = $1; }
;

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /* End of ProgramBody */
    END {
        root = new ProgramNode(@1.first_line, @1.first_column,
                               $1, $3, $4, $5);

        free($1);
    }
;

ProgramName:
    ID
;

DeclarationList:
    Epsilon { $$ = NULL; }
    |
    Declarations { $$ = $1; }
;

Declarations:
    Declaration { 
        $$ = new std::vector<DeclNode *>(); 
        $$->push_back($1);
    }
    |
    Declarations Declaration {
        $$ = $1;
        $$->push_back($2);
    }
;

FunctionList:
    Epsilon { $$ = NULL; }
    |
    Functions { $$ = $1; }
;

Functions:
    Function {
        $$ = new std::vector<FunctionNode *>();
        $$->push_back($1);
    }
    |
    Functions Function {
        $$ = $1;
        $$->push_back($2);
    }
;

Function:
    FunctionDeclaration { $$ = $1; }
    |
    FunctionDefinition { $$ = $1; }
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON {
        if ($3 != NULL)
        {
            std::vector<DeclNode *> *declaration_list = new std::vector<DeclNode *>();
            for(auto formal_argument : *$3)
            {
                DeclNode *decl_formal_arg = (DeclNode *)formal_argument;
                declaration_list->push_back(decl_formal_arg);
            }
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, declaration_list, $5, NULL);
        }
        else
        {
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, NULL, $5, NULL);
        }
    }
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END {
        if ($3 != NULL)
        {
            std::vector<DeclNode *> *declaration_list = new std::vector<DeclNode *>();
            for(auto formal_argument : *$3)
            {
                DeclNode *decl_formal_arg = (DeclNode *)formal_argument;
                declaration_list->push_back(decl_formal_arg);
            }
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, declaration_list, $5, $6);
        }
        else
        {
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, NULL, $5, $6);
        }
    }
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon { $$ = NULL; }
    |
    FormalArgs { $$ = $1; }
;

FormalArgs:
    FormalArg {
        $$ = new std::vector<DeclNode *>();
        $$->push_back($1);
    }
    |
    FormalArgs SEMICOLON FormalArg {
        $$ = $1;
        $$->push_back($3);
    }
;

FormalArg:
    IdList COLON Type {
        std::vector<VariableNode *> *variable_list = new std::vector<VariableNode *>();

        for (auto id : *$1) {
            variable_list->push_back(new VariableNode(id->line, id->column, id->val.c_str(), $3, NULL));
        }

        $$ = new DeclNode(@1.first_line, @1.first_column, variable_list);
    }
;

IdList:
    ID {
        $$ = new std::vector<Variable *>();
        $$->push_back(new Variable($1, @1.first_line, @1.first_column));
    }
    |
    IdList COMMA ID {
        $$ = $1;
        $$->push_back(new Variable($3, @3.first_line, @3.first_column));
    }
;

ReturnType:
    COLON ScalarType { $$ = (char *)$2; }
    |
    Epsilon { $$ = (char *)"void"; }
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON {
        // for id list
        std::vector<VariableNode *> *id_list_type = new std::vector<VariableNode *>();

        for (auto id : *$2)
        {
            id_list_type->push_back(new VariableNode(id->line, id->column, id->val.c_str(), $4, NULL));
        }

        $$ = new DeclNode(@1.first_line, @1.first_column, id_list_type);
    }
    |
    VAR IdList COLON LiteralConstant SEMICOLON {
        std::vector<VariableNode *> *id_list_type = new std::vector<VariableNode *>();
        ConstantValueNode *constant_value = new ConstantValueNode(@1.first_line, $4->column, $4->val.c_str());

        for (auto id : *$2)
        {
            id_list_type->push_back(new VariableNode(id->line, id->column, id->val.c_str(), $4->type.c_str(), constant_value));
        }

        $$ = new DeclNode(@1.first_line, @1.first_column, id_list_type);
    }
;

Type:
    ScalarType { $$ = $1; }
    |
    ArrType { $$ = $1; }
;

ScalarType:
    INTEGER { $$ = (char *)"integer"; }
    |
    REAL { $$ = (char *)"real"; }
    |
    STRING { $$ = (char *)"string"; }
    |
    BOOLEAN { $$ = (char *)"boolean"; }
;

ArrType:
    ArrDecl ScalarType {
        std::string scalar = std::string($2);
        std::string arr = std::string($1);
        $$ = (char *)(scalar + ' ' + arr).c_str();
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF {
        std::string int_literal = std::to_string($2);
        $$ = (char *)('[' + int_literal + ']').c_str();
    }
    |
    ArrDecl ARRAY INT_LITERAL OF {
        std::string int_literal = std::to_string($3);
        std::string arr_dec = std::string($1);
        $$ = (char *)(arr_dec + '[' + int_literal + ']').c_str();
    }
;

LiteralConstant:
    NegOrNot INT_LITERAL {
        if($1 == 1)
        {
            std::string value = std::to_string($2);
            $$ = new Variable("integer", value);
            $$->column = @2.first_column;
        }
        else
        {
            int val = $2 * (-1);
            std::string value = std::to_string(val);
            $$ = new Variable("integer", value);
            $$->column = @1.first_column;
        }
    }
    |
    NegOrNot REAL_LITERAL {
        if($1 == 1)
        {
            std::string value = std::to_string($2);
            $$ = new Variable("real", value);
            $$->column = @2.first_column;
        }
        else
        {
            double val = $2 * (double) (-1.0);
            std::string value = std::to_string(val);
            $$ = new Variable("real", value);
            $$->column = @1.first_column;
        }
    }
    |
    StringAndBoolean { $$ = $1; }
;

NegOrNot:
    Epsilon { $$ = 1; }
    |
    MINUS %prec UNARY_MINUS { $$ = -1; }
;

StringAndBoolean:
    STRING_LITERAL {
        $$ = new Variable("string", $1);
        $$->column = @1.first_column;
    }
    |
    TRUE {
        $$ = new Variable("boolean", "true");
        $$->column = @1.first_column;
    }
    |
    FALSE {
        $$ = new Variable("boolean", "false");
        $$->column = @1.first_column;
    }
;

IntegerAndReal:
    INT_LITERAL { $$ = (char *)(std::to_string($1)).c_str(); }
    |
    REAL_LITERAL { $$ = (char *)(std::to_string($1)).c_str(); }
;

    /*
       Statements
                  */

Statement:
    CompoundStatement { $$ = $1; }
    |
    Simple { $$ = $1; }
    |
    Condition { $$ = $1; }
    |
    While { $$ = $1; }
    |
    For { $$ = $1; }
    |
    Return { $$ = $1; }
    |
    FunctionCall { $$ = $1; }
;

CompoundStatement:
    BEGIN
    DeclarationList
    StatementList
    END {
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, $2, $3);
    }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON { $$ = new AssignmentNode(@2.first_line, @2.first_column, $1, $3); }
    |
    PRINT Expression SEMICOLON { $$ = new PrintNode(@1.first_line, @1.first_column, $2); }
    |
    READ VariableReference SEMICOLON { $$ = new ReadNode(@1.first_line, @1.first_column, $2); }
;

VariableReference:
    ID ArrRefList { $$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1, $2); }
;

ArrRefList:
    Epsilon { $$ = NULL; }
    |
    ArrRefs { $$ = $1; }
;

ArrRefs:
    L_BRACKET Expression R_BRACKET {
        $$ = new std::vector<ExpressionNode *>();
        $$->push_back($2);
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET {
        $$ = $1;
        $$->push_back($3);
    }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF { $$ = new IfNode(@1.first_line, @1.first_column, $2, $4, $5); }
;

ElseOrNot:
    ELSE
    CompoundStatement { $$ = $2; }
    |
    Epsilon { $$ = NULL; }
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO { $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4); }
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO {
        VariableNode *tmp_var = new VariableNode(@2.first_line, @2.first_column, $2, "integer", NULL);
        std::vector<VariableNode *> *tmp_var_list = new std::vector<VariableNode *>({tmp_var});
        VariableReferenceNode *tmp_var_re = new VariableReferenceNode(@1.first_line, @2.first_column, $2, NULL);
        ConstantValueNode *tmp_ass_const = new ConstantValueNode(@1.first_line, @4.first_column, std::to_string($4).c_str());
        
        DeclNode *declaration = new DeclNode(@1.first_line, @2.first_column, tmp_var_list);
        AssignmentNode *assignment = new AssignmentNode(@1.first_line, @3.first_column, tmp_var_re, tmp_ass_const);                                             
        ExpressionNode *expression = new ConstantValueNode(@1.first_line, @6.first_column, std::to_string($6).c_str());
        
        $$ = new ForNode(@1.first_line, @1.first_column, declaration, assignment, expression, $8);
    }
;

Return:
    RETURN Expression SEMICOLON { $$ = new ReturnNode(@1.first_line, @1.first_column, $2); }
;

FunctionCall:
    FunctionInvocation SEMICOLON { $$ = $1; }
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS { $$ = new FunctionInvocationNode(@1.first_line, @1.first_column, $1, $3); }
;

ExpressionList:
    Epsilon { $$ = NULL; } 
    |
    Expressions { $$ = $1; }
;

Expressions:
    Expression {
        $$ = new std::vector<ExpressionNode *>();
        $$->push_back($1);
    }
    |
    Expressions COMMA Expression {
        $$ = $1;
        $$->push_back($3);
    }
;

StatementList:
    Epsilon { $$ = NULL; }
    |
    Statements { $$ = $1; }
;

Statements:
    Statement {
        $$ = new std::vector<AstNode *>();
        $$->push_back($1);
    }
    |
    Statements Statement {
        $$ = $1;
        $$->push_back($2);
    }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS { $$ = $2; }
    |
    MINUS Expression %prec UNARY_MINUS { $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, "neg", $2); }
    |
    Expression MULTIPLY Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "*", $1, $3); }
    |
    Expression DIVIDE Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "/", $1, $3); }
    |
    Expression MOD Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "mod", $1, $3); }
    |
    Expression PLUS Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "+", $1, $3); }
    |
    Expression MINUS Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "-", $1, $3); }
    |
    Expression LESS Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<", $1, $3); }
    |
    Expression LESS_OR_EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<=", $1, $3); }
    |
    Expression GREATER Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, ">", $1, $3); }
    |
    Expression GREATER_OR_EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, ">=", $1, $3); }
    |
    Expression EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "=", $1, $3); }
    |
    Expression NOT_EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<>", $1, $3); }
    |
    NOT Expression { $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, "not", $2); }
    |
    Expression AND Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "and", $1, $3); }
    |
    Expression OR Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "or", $1, $3); }
    |
    IntegerAndReal { $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1); }
    |
    StringAndBoolean { $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1->val.c_str()); }
    |
    VariableReference { $$ = $1; }
    |
    FunctionInvocation { $$ = $1; }
;

    /*
       misc
            */
Epsilon:
;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [--dump-ast]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        AstDumper ast_dumper;
        root->accept(ast_dumper);
    }

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
