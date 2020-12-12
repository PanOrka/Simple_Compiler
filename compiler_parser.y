%{
#include <stdio.h>

int yylex();
extern FILE *yyin;
int yyerror(char*);

FILE *compiler_out;
%}

/*//////////////////////////////////////////////
//
// TOKENS
//
*///////////////////////////////////////////////

%token END

/*
// SYMBOL & CONST VALUE
*/
%token pidentifier // make it struct or union or w/e
%token num

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
