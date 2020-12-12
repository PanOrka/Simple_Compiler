%option noyywrap

%{
#include "prelexer.h"

int zzlex();
%}

%%
"END" {
    printf("END\n");
}

[ \t]+ ;

[\n]+ ;

. {
    printf("%s", zztext);
}
%%

void prelex(FILE *in, FILE *out) {
    zzlex();
}
