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
%token BEGIN_P
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
// READ & WRITE
*/
%token READ
%token WRITE

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

%token COMA // ,

/*
// SYMBOL & CONST VALUE
*/
%token <id> pidentifier
%token <value> num

/*//////////////////////////////////////////////
//
// RULES
//
*//////////////////////////////////////////////
%%
line: %empty
    | any ENDL line
;

any: DECLARE declarations
   | BEGIN_P
   | identifier ASSIGN expression END_EXPR
   | IF condition THEN
   | ELSE
   | ENDIF
   | WHILE condition DO
   | ENDWHILE
   | REPEAT
   | UNTIL condition END_EXPR
   | FOR pidentifier FROM value TO value DO
   | FOR pidentifier FROM value DOWNTO value DO
   | ENDFOR
   | READ identifier END_EXPR
   | WRITE value END_EXPR
   | END
;

declarations: declarations COMA pidentifier
            | declarations COMA pidentifier L_BRACE num ARRAY_IND num R_BRACE
            | pidentifier
            | pidentifier L_BRACE num ARRAY_IND num R_BRACE

expression: value
          | value ADD value
          | value SUB value
          | value MUL value
          | value DIV value
          | value MOD value
;

condition: value IS_EQUAL value
         | value IS_N_EQUAL value
         | value LESS value
         | value GREATER value
         | value LESS_EQ value
         | value GREATER_EQ value
;

value: num
     | identifier
;

identifier: pidentifier
          | pidentifier L_BRACE pidentifier R_BRACE
          | pidentifier L_BRACE num R_BRACE
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
