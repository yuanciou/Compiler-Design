%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LEN 512
/* Code runs each time a token is matched. */
#define YY_USER_ACTION updateCurrentLine(yytext);

uint32_t line_num = 1;
uint32_t col_num = 1;
static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char current_line[MAX_LINE_LEN];

static void updateCurrentLine(const char *source);
static void listToken(const char *name);
static void listLiteral(const char *name, const char *literal);

%}

/* Homework start */
    /* Definition (character set) */
digit [0-9]
space [ \t\n]+
letter [a-zA-Z]
id {letter}({letter}|{digit})*
integer_oct 0[0-7]+
integer_dec 0|([1-9]{digit}*)
float {integer_dec}\.(0|{digit}*[1-9])
scientific (([1-9]{digit}*)|(([1-9]{digit}*)\.(0|{digit}*[1-9])))[eE][+-]?{integer_dec}
string \"([^\"\n]|\"\")*\"

%x C_style_comment
%x CPP_style_comment

%%
    /* Delimiters */
"," {listToken(",");}
";" {listToken(";");}
":" {listToken(":");}
"(" {listToken("(");}
")" {listToken(")");}
"[" {listToken("[");}
"]" {listToken("]");}

    /* Arithmetic, Relational, and Logical Operators */
"+"   {listToken("+");}
"-"   {listToken("-");}
"*"   {listToken("*");}
"/"   {listToken("/");}
"mod" {listToken("mod");}
":="  {listToken(":=");}
"<"   {listToken("<");}
"<="  {listToken("<=");}
"<>"  {listToken("<>");}
">="  {listToken(">=");}
">"   {listToken(">");}
"="   {listToken("=");}
"and" {listToken("and");}
"or"  {listToken("or");}
"not" {listToken("not");}

    /* Reserved Words */
        /*Declaration*/
"var"     {listToken("KWvar");}
"def"     {listToken("KWdef");}
        /*Type*/
"array"   {listToken("KWarray");}
"of"      {listToken("KWof");}
"boolean" {listToken("KWboolean");}
"integer" {listToken("KWinteger");}
"real"    {listToken("KWreal");}
"string"  {listToken("KWstring");}
        /*Value*/
"true"    {listToken("KWtrue");}
"false"   {listToken("KWfalse");}
        /*Flow-of-control*/
"while"   {listToken("KWwhile");}
"do"      {listToken("KWdo");}
"if"      {listToken("KWif");}
"then"    {listToken("KWthen");}
"else"    {listToken("KWelse");}
"for"     {listToken("KWfor");}
"to"      {listToken("KWto");}
        /*Block*/
"begin"   {listToken("KWbegin");}
"end"     {listToken("KWend");}
        /*Statement*/
"print"   {listToken("KWprint");}
"read"    {listToken("KWread");}
"return"  {listToken("KWreturn");}

    /* Identifiers */
{id} {listLiteral("id", yytext);}

    /* Integer Constants */
{integer_dec} {listLiteral("integer", yytext);}
{integer_oct} {listLiteral("oct_integer", yytext);}

    /* Floating-Point Constants */
{float} {listLiteral("float", yytext);}

    /* Scientific Notations */
{scientific} {listLiteral("scientific", yytext);}

    /* String Constants */
{string} {
    int yyleng = strlen(yytext);
    char* tmp = malloc(yyleng + 1);
    strcpy(tmp, yytext);
    if(opt_tok) 
    {
        printf("<string: ");
    }
    tmp[yyleng - 1] = '\0';

    for(int i = 1; i < strlen(tmp); i++)
    {
        if(i + 1 < strlen(tmp) && tmp[i] == '"' && tmp[i+1] == '"')
        {
            if(opt_tok)
            {
                printf("%c", tmp[i]);
            }
            i++;
            continue;
        }
        if(opt_tok)
        {
            printf("%c", tmp[i]);
        }
    }

    if(opt_tok) 
    {
        printf(">\n");
    }
}

    /* Whitespace */
{space} {}

    /* Pseudocomments */
"//&S-"(.*) { opt_src = 0; }
"//&T-"(.*) { opt_tok = 0; }
"//&S+"(.*) { opt_src = 1; }
"//&T+"(.*) { opt_tok = 1; }

    /* Comments */
        /*C-style*/
<INITIAL,C_style_comment>\n {}
"/*" {BEGIN C_style_comment;}
<C_style_comment>. {}
<C_style_comment>"*/" {BEGIN INITIAL;}

        /*C++-style*/
<CPP_style_comment>\n {BEGIN INITIAL;}
"//" { BEGIN CPP_style_comment;}
<CPP_style_comment>. { }

 "(" { listToken("("); }
    /* Catch the character which is not accepted by rules above */
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

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    fclose(yyin);
    /* Free resources used by the scanner; otherwise, unused data structures
    will persist throughout the successive compiler phases. */
    yylex_destroy();

    return 0;
}
