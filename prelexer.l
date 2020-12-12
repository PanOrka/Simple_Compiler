%option noyywrap

%{
#include <stdio.h>

int zzlex();
void prelex(FILE *in, FILE *out);
%}

%%
<INITIAL>{
    "END" {
        printf("END\n");
    }

    [ \t]+ ;

    [\n]+ ;

    . {
        printf("%s", yylex());
    }
}
%%

void prelex(FILE *in, FILE *out) {
    zzlex();
}
