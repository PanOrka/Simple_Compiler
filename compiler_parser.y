%{
#include <stdio.h>
#include <stdint.h>

int yylex();
extern FILE *yyin;
int yyerror(char*);

FILE *compiler_out;
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
%token BEGIN
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
line: expr '\n'
;

expr: END {
    printf("END\n");
}
;
%%

int yyerror(char *s) {
    printf("%s\n",s);
    return 0;
}

void parse(FILE *in, FILE *out) {
    yyin = in;
    compiler_out = out;
    yyparse();
}
