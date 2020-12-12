%option noyywrap

%{
#include "compiler_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex();
%}

%%

"DECLARE" {
    return DECLARE;
}


"BEGIN" {
    return _BEGIN;
}

"END" {
    return END;
}


"IF" {
    return IF;
}

"THEN" {
    return THEN;
}

"ELSE" {
    return ELSE;
}

"ENDIF" {
    return ENDIF;
}


"WHILE" {
    return WHILE;
}

"ENDWHILE" {
    return ENDWHILE;
}


"REPEAT" {
    return REPEAT;
}

"UNTIL" {
    return UNTIL;
}


"FOR" {
    return FOR;
}

"FROM" {
    return FROM;
}

"TO" {
    return TO;
}

"DOWNTO" {
    return DOWNTO;
}

"ENDFOR" {
    return ENDFOR;
}


"DO" {
    return DO;
}

[_a-z]+ {
    size_t len = strlen(yytext) + 1;
    char *id = malloc(len);
    if (id == NULL) {
        fprintf(stderr, "Couldn't alocate id of pidentifier: %s!", yytext);
        exit(EXIT_FAILURE);
    }
    yylval.id = strcpy(id, yytext);

    return pidentifier;
}

[0-9]+ {
    yylval.value = strtoll(yytext, NULL, 10);

    return num;
}
%%
