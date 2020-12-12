%option noyywrap

%{
#include "prelexer.h"

int zzlex();

FILE *_out;

%}

%s DECLARE

%%
<INITIAL>{
    "DECLARE" {
        fprintf(_out, "DECLARE");
        BEGIN(DECLARE);
    }

    "BEGIN" {
        fprintf(_out, "BEGIN\n");
    }

    ";" {
        fprintf(_out, ";\n");
    }

    "THEN" {
        fprintf(_out, "THEN\n");
    }

    "ELSE" {
        fprintf(_out, "ELSE\n");
    }

    "DOWNTO" {
        fprintf(_out, "DOWNTO");
    }

    "DO" {
        fprintf(_out, "DO\n");
    }

    "REPEAT" {
        fprintf(_out, "REPEAT\n");
    }

    "ENDIF" {
        fprintf(_out, "ENDIF\n");
    }

    "ENDWHILE" {
        fprintf(_out, "ENDWHILE\n");
    }

    "ENDFOR" {
        fprintf(_out, "ENDFOR\n");
    }

    "END" {
        fprintf(_out, "END\n");
    }

    [ \t]+ ;

    [\n]+ ;

    . {
        fprintf(_out, "%s", zztext);
    }
}

<DECLARE>{
    "BEGIN" {
        fprintf(_out, "\nBEGIN\n");
        BEGIN(INITIAL);
    }

    [ \t]+ ;

    [\n]+ ;

    . {
        fprintf(_out, "%s", zztext);
    }
}
%%

void prelex(FILE *in, FILE *out) {
    _out = out;
    zzin = in;
    zzlex();
}
