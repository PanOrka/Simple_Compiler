%{
#include <stdio.h>
#include <stdint.h>

int yylex();
extern FILE *yyin;
int yyerror(char*);

static FILE *compiler_out;
%}

%union {
    char *id;
    int64_t value;
};

/*//////////////////////////////////////////////
//
// TOKENS
//
*///////////////////////////////////////////////

/*
// DECLARE
*/
%token DECLARE

/*
// BODY
*/
%token _BEGIN
%token END

/*
// IF
*/
%token IF
%token THEN
%token ELSE
%token ENDIF

/*
// WHILE
*/
%token WHILE
%token ENDWHILE

/*
// REPEAT
*/
%token REPEAT
%token UNTIL

/*
// FOR
*/
%token FOR
%token FROM
%token TO
%token DOWNTO
%token ENDFOR

/*
// UNIVERSAL
*/
%token DO

/*
// ENDL
*/
%token ENDL

/*
// OPERATORS & Extra Symbols
*/
%token ASSIGN // :=
%token END_EXPR // ;

%token ADD // +
%token SUB // -
%token MUL // *
%token DIV // /
%token MOD // %

%token IS_EQUAL // =
%token IS_N_EQUAL // !=

%token LESS // <
%token GREATER // >
%token LESS_EQ // <=
%token GREATER_EQ // >=

%token L_BRACE // (
%token R_BRACE // )
%token ARRAY_IND // :

/*
// SYMBOL & CONST VALUE
*/
%token <id> pidentifier // make it struct or union or w/e
%token <value> num

/*//////////////////////////////////////////////
//
// RULES
//
*//////////////////////////////////////////////
%%
line: %empty
    | expr ENDL line
;

expr: END {
        printf("END\n");
    }
    | DO {
        printf("DO\n");
    }
;
%%

int yyerror(char *s) {
    printf("%s\n", s);
    return 0;
}

void parse(FILE *in, FILE *out) {
    yyin = in;
    compiler_out = out;
    yyparse();
}
