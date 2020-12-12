%{
#include <stdio.h>

int yylex();
int yyerror(char*);
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
