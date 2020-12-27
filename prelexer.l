%option noyywrap

%{
#include "prelexer.h"

int zzlex();

static FILE *prelexer_out;
%}

%s DECLARE
%s COMMENT_DECL
%s COMMENT

%%
<INITIAL>{
    "DECLARE" {
        fprintf(prelexer_out, "DECLARE");
        BEGIN(DECLARE);
    }

    "BEGIN" {
        fprintf(prelexer_out, "BEGIN\n");
    }

    ";" {
        fprintf(prelexer_out, ";\n");
    }

    "THEN" {
        fprintf(prelexer_out, "THEN\n");
    }

    "ELSE" {
        fprintf(prelexer_out, "ELSE\n");
    }

    "DOWNTO" {
        fprintf(prelexer_out, "DOWNTO");
    }

    "DO" {
        fprintf(prelexer_out, "DO\n");
    }

    "REPEAT" {
        fprintf(prelexer_out, "REPEAT\n");
    }

    "ENDIF" {
        fprintf(prelexer_out, "ENDIF\n");
    }

    "ENDWHILE" {
        fprintf(prelexer_out, "ENDWHILE\n");
    }

    "ENDFOR" {
        fprintf(prelexer_out, "ENDFOR\n");
    }

    "END" {
        fprintf(prelexer_out, "END\n");
    }

    "[" {
        BEGIN(COMMENT);
    }

    [ \t]+ ;

    [\n]+ ;

    . {
        fprintf(prelexer_out, "%s", zztext);
    }
}

<DECLARE>{
    "BEGIN" {
        fprintf(prelexer_out, "\nBEGIN\n");
        BEGIN(INITIAL);
    }

    "[" {
        BEGIN(COMMENT_DECL);
    }

    [ \t]+ ;

    [\n]+ ;

    . {
        fprintf(prelexer_out, "%s", zztext);
    }
}

<COMMENT_DECL>{
    "]" {
        BEGIN(DECLARE);
    }

    [\n]+ {
        fprintf(stderr, "[PRELEXER]: Comments should be one liners!\n");
        exit(EXIT_FAILURE);
    }

    . ;
}

<COMMENT>{
    "]" {
        BEGIN(INITIAL);
    }

    [\n]+ {
        fprintf(stderr, "[PRELEXER]: Comments should be one liners!\n");
        exit(EXIT_FAILURE);
    }

    . ;
}
%%

void prelex(FILE *in, FILE *out) {
    prelexer_out = out;
    zzin = in;
    zzlex();
}
