%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define MAX_LINE_LEN 512
#define MAX_ID_LEN 32
#define MAX_LINE_NUM 200
/* Code runs each time a token is matched. */
#define YY_USER_ACTION \
    yylloc.first_line = line_num; \
    yylloc.first_column = col_num; \
    updateCurrentLine(yytext);

/* prevent undefined reference error in newer version of flex */
extern "C" int yylex(void);

uint32_t line_num = 1;
uint32_t col_num = 1;
long line_positions[MAX_LINE_NUM + 1] = {0}; // +1 since we use 1-based
char current_line[MAX_LINE_LEN];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
uint32_t opt_dmp = 1;
static char string_literal[MAX_LINE_LEN];

static void updateCurrentLine(const char *source);
static void listToken(const char *name);
static void listLiteral(const char *name, const char *literal);

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
"," { listToken(","); return TOK_COMMA; }
";" { listToken(";"); return TOK_SEMICOLON; }
":" { listToken(":"); return TOK_COLON; }
"(" { listToken("("); return TOK_L_PARENTHESIS; }
")" { listToken(")"); return TOK_R_PARENTHESIS; }
"[" { listToken("["); return TOK_L_BRACKET; }
"]" { listToken("]"); return TOK_R_BRACKET; }

    /* Operator */
"+"   { listToken("+"); return TOK_PLUS; }
"-"   { listToken("-"); return TOK_MINUS; }
"*"   { listToken("*"); return TOK_MULTIPLY; }
"/"   { listToken("/"); return TOK_DIVIDE; }
"mod" { listToken("mod"); return TOK_MOD; }
":="  { listToken(":="); return TOK_ASSIGN; }
"<"   { listToken("<"); return TOK_LESS; }
"<="  { listToken("<="); return TOK_LESS_OR_EQUAL; }
"<>"  { listToken("<>"); return TOK_NOT_EQUAL; }
">="  { listToken(">="); return TOK_GREATER_OR_EQUAL; }
">"   { listToken(">"); return TOK_GREATER; }
"="   { listToken("="); return TOK_EQUAL; }
"and" { listToken("and"); return TOK_AND; }
"or"  { listToken("or"); return TOK_OR; }
"not" { listToken("not"); return TOK_NOT; }

    /* Reserved Word */
"var"     { listToken("KWvar"); return TOK_VAR; }
"array"   { listToken("KWarray"); return TOK_ARRAY; }
"of"      { listToken("KWof"); return TOK_OF; }
"boolean" { listToken("KWboolean"); return TOK_BOOLEAN; }
"integer" { listToken("KWinteger"); return TOK_INTEGER; }
"real"    { listToken("KWreal"); return TOK_REAL; }
"string"  { listToken("KWstring"); return TOK_STRING; }

"true"    {
    listToken("KWtrue");
    yylval.boolean = true;
    return TOK_TRUE;
}
"false"   {
    listToken("KWfalse");
    yylval.boolean = false;
    return TOK_FALSE;
}

"def"     { listToken("KWdef"); return TOK_DEF; }
"return"  { listToken("KWreturn"); return TOK_RETURN; }

"begin"   { listToken("KWbegin"); return TOK_BEGIN; }
"end"     { listToken("KWend"); return TOK_END; }

"while"   { listToken("KWwhile"); return TOK_WHILE; }
"do"      { listToken("KWdo"); return TOK_DO; }

"if"      { listToken("KWif"); return TOK_IF; }
"then"    { listToken("KWthen"); return TOK_THEN; }
"else"    { listToken("KWelse"); return TOK_ELSE; }

"for"     { listToken("KWfor"); return TOK_FOR; }
"to"      { listToken("KWto"); return TOK_TO; }

"print"   { listToken("KWprint"); return TOK_PRINT; }
"read"    { listToken("KWread"); return TOK_READ; }

    /* Identifier */
[a-zA-Z][a-zA-Z0-9]* {
    listLiteral("id", yytext);
    yylval.identifier = strndup(yytext, MAX_ID_LEN);
    return TOK_ID;
}

    /* Integer (decimal/octal) */
{integer} {
    listLiteral("integer", yytext);
    yylval.integer = strtol(yytext, NULL, 10);
    return TOK_INT_LITERAL;
}
0[0-7]+   {
    listLiteral("oct_integer", yytext);
    yylval.integer = strtol(yytext, NULL, 8);
    return TOK_INT_LITERAL;
}

    /* Floating-Point */
{float} {
    listLiteral("float", yytext);
    yylval.real = atof(yytext);
    return TOK_REAL_LITERAL;
}

    /* Scientific Notation [Ee][+-]?[0-9]+ */
({nonzero_integer}|{nonzero_float})[Ee][+-]?({integer}) {
    listLiteral("scientific", yytext);
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
    listLiteral("string", string_literal);
    yylval.string = strdup(string_literal);
    return TOK_STRING_LITERAL;
}

    /* Whitespace and Newline */
[ \t\n]+ { }

    /* Pseudocomment */
"//&"[STD][+-].* {
    char option = yytext[3];
    switch (option) {
    case 'S':
        opt_src = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'T':
        opt_tok = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'D':
        opt_dmp = (yytext[4] == '+') ? 1 : 0;
        break;
    }
}

    /* C++ Style Comment */
"//".* { }

    /* C Style Comment */
"/*"           { BEGIN(CCOMMENT); }
<CCOMMENT>"*/" { BEGIN(INITIAL); }
<CCOMMENT>.    { }

    /* Catch the character which is not accepted by all rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%

/** @note The line is printed out and flushed when a newline character is encountered. */
static void updateCurrentLine(const char *source) {
    /* col_num is one-based */
    for (const char *c = source; *c; ++c) {
        if (*c == '\n') {
            if (opt_src) {
                printf("%d: %s\n", line_num, current_line);
            }
            line_positions[line_num + 1] = line_positions[line_num] + col_num;
            ++line_num;
            col_num = 1;
            current_line[0] = '\0';
        } else {
            if (col_num < MAX_LINE_LEN) {
                current_line[col_num - 1] = *c;
                current_line[col_num] = '\0';
            } else {
                /* Truncate silently; doesn't affect the program's correctness. */
            }
            ++col_num;
        }
    }
}

static void listToken(const char *name) {
    if (opt_tok) {
        printf("<%s>\n", name);
    }
}

static void listLiteral(const char *name, const char *literal) {
    if (opt_tok) {
        printf("<%s: %s>\n", name, literal);
    }
}

/** @note This function is not required if the input file is guaranteed to end
 * with a newline. However, students may find it useful to handle the case where
 * the input file does not end with a newline, as it has been reported several
 * times in the past.
 */
int yywrap(void) {
    /* If the file is not ended with a newline, fake it to print out the last line. */
    if (col_num > 1) {
        updateCurrentLine("\n");
    }
    /* no more input file */
    return 1;
}
