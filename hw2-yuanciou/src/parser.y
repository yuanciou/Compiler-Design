%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%token MOD ASSIGN LE NE GE AND OR NOT

%token KWvar KWarray KWof KWboolean KWinteger KWreal KWstring
%token KWtrue KWfalse
%token KWdef KWreturn 
%token KWbegin KWend 
%token KWwhile KWdo 
%token KWif KWthen KWelse 
%token KWfor KWto 
%token KWprint KWread

%token ID Int Oct_int Float Sci Str

%left AND OR NOT
%left '<' LE GE NE '>' '='
%left '-'
%left '+'
%left '/' MOD
%left '*'

%%

/* Program */
ProgramName: ID ';' Declaration Function Compound_S KWend;

/* Function */
    /* function header */
Function_Header: ID '(' Formal_Arguments ')' ':' Scalar_Type
               | ID '(' Formal_Arguments ')';
    /* funciton declaration */
Function_Declaration: Function_Header ';';
    /* function definition */
Function_Definition: Function_Header Compound_S KWend;   
    /* functions */
Function: 
         | Function_Declaration Function
         | Function_Definition Function;

/* Formal Argument */
Formal_Argument: Id_List ':' Type;
Formal_Arguments:
                | Formal_Argument ';' Formal_Arguments
                | Formal_Argument;

/* Declaration */
Declaration:
           | Variable_Declaration Declaration
           | Constant_Declaration Declaration;

Variable_Declaration: KWvar Id_List ':' Type ';';

Constant_Declaration: KWvar Id_List ':' Literal_Constant ';'
                    | KWvar Id_List ':' '-' Integer_Literal ';'
                    | KWvar Id_List ':' '-' Real_Literal ';';

Id_List: ID | ID ',' Id_List;

/* Types */
Type: Scalar_Type | Array_Type;

Scalar_Type: KWinteger | KWreal | KWstring | KWboolean;

Array_Type: KWarray Integer_Literal KWof Scalar_Type 
          | KWarray Integer_Literal KWof Array_Type;

/* Statements */
Statement: | Statement Simple_S | Statement Conditional_S | Statement Function_Call_S 
           | Statement Loop_S | Statement Return_S | Statement Compound_S;

Simple_S: Variable_Reference ASSIGN Expression ';'
        | KWprint Expression ';'
        | KWread Variable_Reference ';';
    /* condition statement */
Conditional_S: KWif Expression KWthen Compound_S KWend KWif
             | KWif Expression KWthen Compound_S KWelse Compound_S KWend KWif;

Function_Call_S: Function_Call ';';

Loop_S: While_S | For_S;
While_S: KWwhile Expression KWdo Compound_S KWend KWdo
For_S: KWfor ID ASSIGN Integer_Literal KWto Integer_Literal KWdo Compound_S KWend KWdo

Return_S: KWreturn Expression ';';

Compound_S: KWbegin Declaration Statement KWend;

/* Expression */
Expression: Literal_Constant | Variable_Reference | Function_Call
          | Binary_Operation | Unary_Operation | '(' Expression ')';

    /* literal constant */
Literal_Constant: Integer_Literal | Real_Literal | String_Literal | Boolean_literal;

Integer_Literal: Int | Oct_int;
Real_Literal: Float | Sci;
String_Literal: Str;
Boolean_literal: KWtrue | KWfalse;

    /* variable reference */
Variable_Reference: ID | Array_Reference;
Array_Reference: ID '[' Expression ']' | Array_Reference '[' Expression ']';

    /* function call */
Function_Call: ID '(' Func_Expr ')';
Func_Expr:
         | Expression | Func_Expr ',' Expression;

Binary_Operation: Expression '+' Expression
                | Expression '-' Expression
                | Expression '*' Expression
                | Expression '/' Expression
                | Expression MOD Expression
                | Expression '<' Expression
                | Expression LE Expression
                | Expression NE Expression
                | Expression GE Expression
                | Expression '>' Expression
                | Expression '=' Expression
                | Expression AND Expression
                | Expression OR Expression;

Unary_Operation: '-' Expression %prec '*'
               | NOT Expression;

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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
