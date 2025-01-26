%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define YY_USER_ACTION \
    yylloc.first_line = line_num; \
    yylloc.first_column = col_num; \
    col_num += yyleng;

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512
#define MAX_ID_LENG                 32
#define MAX_LINE_NUM                200

// prevent undefined reference error in newer version of flex
extern "C" int yylex(void);

uint32_t line_num = 1;
uint32_t col_num = 1;
long line_positions[MAX_LINE_NUM + 1] = {0}; // +1 since we use 1-based
char current_line[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
int32_t opt_dump = 1;
static char string_literal[MAX_LINE_LENG];
static char *current_line_ptr = current_line;

static void appendToCurrentLine(const char *yytext_ptr);

%}

integer 0|[1-9][0-9]*
float {integer}\.(0|[0-9]*[1-9])
nonzero_integer [1-9][0-9]*
    /* If the integral part is nonzero, then the fractional part can be either zero
    or not. However, if the integral part is zero, the fractional part cannot be zero. */
nonzero_float ({nonzero_integer}\.(0|[0-9]*[1-9]))|(0\.([0-9]*[1-9]))

    /* exclusive start condition */
%x CCOMMENT

%%
    /* Delimiter */
"," { LIST_TOKEN(","); return TOK_COMMA; }
";" { LIST_TOKEN(";"); return TOK_SEMICOLON; }
":" { LIST_TOKEN(":"); return TOK_COLON; }
"(" { LIST_TOKEN("("); return TOK_L_PARENTHESIS; }
")" { LIST_TOKEN(")"); return TOK_R_PARENTHESIS; }
"[" { LIST_TOKEN("["); return TOK_L_BRACKET; }
"]" { LIST_TOKEN("]"); return TOK_R_BRACKET; }

    /* Operator */
"+"   { LIST_TOKEN("+"); return TOK_PLUS; }
"-"   { LIST_TOKEN("-"); return TOK_MINUS; }
"*"   { LIST_TOKEN("*"); return TOK_MULTIPLY; }
"/"   { LIST_TOKEN("/"); return TOK_DIVIDE; }
"mod" { LIST_TOKEN("mod"); return TOK_MOD; }
":="  { LIST_TOKEN(":="); return TOK_ASSIGN; }
"<"   { LIST_TOKEN("<"); return TOK_LESS; }
"<="  { LIST_TOKEN("<="); return TOK_LESS_OR_EQUAL; }
"<>"  { LIST_TOKEN("<>"); return TOK_NOT_EQUAL; }
">="  { LIST_TOKEN(">="); return TOK_GREATER_OR_EQUAL; }
">"   { LIST_TOKEN(">"); return TOK_GREATER; }
"="   { LIST_TOKEN("="); return TOK_EQUAL; }
"and" { LIST_TOKEN("and"); return TOK_AND; }
"or"  { LIST_TOKEN("or"); return TOK_OR; }
"not" { LIST_TOKEN("not"); return TOK_NOT; }

    /* Reserved Word */
"var"     { LIST_TOKEN("KWvar"); return TOK_VAR; }
"array"   { LIST_TOKEN("KWarray"); return TOK_ARRAY; }
"of"      { LIST_TOKEN("KWof"); return TOK_OF; }
"boolean" { LIST_TOKEN("KWboolean"); return TOK_BOOLEAN; }
"integer" { LIST_TOKEN("KWinteger"); return TOK_INTEGER; }
"real"    { LIST_TOKEN("KWreal"); return TOK_REAL; }
"string"  { LIST_TOKEN("KWstring"); return TOK_STRING; }

"true"    {
    LIST_TOKEN("KWtrue");
    yylval.boolean = true;
    return TOK_TRUE;
}
"false"   {
    LIST_TOKEN("KWfalse");
    yylval.boolean = false;
    return TOK_FALSE;
}

"def"     { LIST_TOKEN("KWdef"); return TOK_DEF; }
"return"  { LIST_TOKEN("KWreturn"); return TOK_RETURN; }

"begin"   { LIST_TOKEN("KWbegin"); return TOK_BEGIN; }
"end"     { LIST_TOKEN("KWend"); return TOK_END; }

"while"   { LIST_TOKEN("KWwhile"); return TOK_WHILE; }
"do"      { LIST_TOKEN("KWdo"); return TOK_DO; }

"if"      { LIST_TOKEN("KWif"); return TOK_IF; }
"then"    { LIST_TOKEN("KWthen"); return TOK_THEN; }
"else"    { LIST_TOKEN("KWelse"); return TOK_ELSE; }

"for"     { LIST_TOKEN("KWfor"); return TOK_FOR; }
"to"      { LIST_TOKEN("KWto"); return TOK_TO; }

"print"   { LIST_TOKEN("KWprint"); return TOK_PRINT; }
"read"    { LIST_TOKEN("KWread"); return TOK_READ; }

    /* Identifier */
[a-zA-Z][a-zA-Z0-9]* {
    LIST_LITERAL("id", yytext);
    yylval.identifier = strndup(yytext, MAX_ID_LENG);
    return TOK_ID;
}

    /* Integer (decimal/octal) */
{integer} {
    LIST_LITERAL("integer", yytext);
    yylval.integer = strtol(yytext, NULL, 10);
    return TOK_INT_LITERAL;
}
0[0-7]+   {
    LIST_LITERAL("oct_integer", yytext);
    yylval.integer = strtol(yytext, NULL, 8);
    return TOK_INT_LITERAL;
}

    /* Floating-Point */
{float} {
    LIST_LITERAL("float", yytext);
    yylval.real = atof(yytext);
    return TOK_REAL_LITERAL;
}

    /* Scientific Notation [Ee][+-]?[0-9]+ */
({nonzero_integer}|{nonzero_float})[Ee][+-]?({integer}) {
    LIST_LITERAL("scientific", yytext);
    yylval.real = atof(yytext);
    return TOK_REAL_LITERAL;
}

    /* String */
\"([^"\n]|\"\")*\" {
    char *yyt_ptr = yytext + 1;  // +1 for skipping the first double quote "
    char *str_ptr = string_literal;

    while (*yyt_ptr) {
        if (*yyt_ptr == '"') {
            // Handle the situation of two double quotes "" in string literal
            if (*(yyt_ptr + 1) == '"') {
                *str_ptr = *yyt_ptr;
                ++str_ptr;
                yyt_ptr += 2; // move to the next character of ""
            } else {
                ++yyt_ptr;
            }
        } else {  // normal character
            *str_ptr = *yyt_ptr;
            ++str_ptr;
            ++yyt_ptr;
        }
    }
    *str_ptr = '\0';
    LIST_LITERAL("string", string_literal);
    yylval.string = strdup(string_literal);
    return TOK_STRING_LITERAL;
}

    /* Whitespace */
[ \t]+ { LIST_SOURCE; }

    /* Pseudocomment */
"//&"[STD][+-].* {
    LIST_SOURCE;
    char option = yytext[3];
    switch (option) {
    case 'S':
        opt_src = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'T':
        opt_tok = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'D':
        opt_dump = (yytext[4] == '+') ? 1 : 0;
        break;
    }
}

    /* C++ Style Comment */
"//".* { LIST_SOURCE; }

    /* C Style Comment */
"/*"           { LIST_SOURCE; BEGIN(CCOMMENT); }
<CCOMMENT>"*/" { LIST_SOURCE; BEGIN(INITIAL); }
<CCOMMENT>.    { LIST_SOURCE; }

    /* Newline */
<INITIAL,CCOMMENT>\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, current_line);
    }
    line_positions[line_num + 1] = line_positions[line_num] +
                                   (current_line_ptr - current_line + 1);
    ++line_num;
    col_num = 1;
    current_line[0] = '\0';
    current_line_ptr = current_line;
}

    /* Catch the character which is not accepted by all rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%

static void appendToCurrentLine(const char *yytext_ptr) {
    while (*yytext_ptr) {
        *current_line_ptr = *yytext_ptr;
        ++current_line_ptr;
        ++yytext_ptr;
    }
    *current_line_ptr = '\0';
}
