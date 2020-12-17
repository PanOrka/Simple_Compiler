%{
#include <stdio.h>
#include <stdint.h>

int yylex();
extern FILE *yyin;
int yyerror(char*);

static FILE *compiler_out;

#include "parser_func/declarations.h"
#include "parser_func/expressions.h"
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
%left ASSIGN // :=
%token END_EXPR // ;

%left ADD // +
%left SUB // -
%left MUL // *
%left DIV // /
%left MOD // %

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
   | identifier ASSIGN expression END_EXPR {
       expression_t expr = expression_get();
       print_expression(&expr);
   }
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

declarations: declarations COMA pidentifier {
                declare_var($3);
            }
            | declarations COMA pidentifier L_BRACE num ARRAY_IND num R_BRACE {
                declare_array($3, $5, $7);
            }
            | pidentifier {
                declare_var($1);
            }
            | pidentifier L_BRACE num ARRAY_IND num R_BRACE {
                declare_array($1, $3, $5);
            }
;

expression: value {
              expression_set_type((expr_type)expr_VALUE);
          }
          | value ADD value {
              expression_set_type((expr_type)expr_ADD);
          }
          | value SUB value {
              expression_set_type((expr_type)expr_SUB);
          }
          | value MUL value {
              expression_set_type((expr_type)expr_MUL);
          }
          | value DIV value {
              expression_set_type((expr_type)expr_DIV);
          }
          | value MOD value {
              expression_set_type((expr_type)expr_MOD);
          }
;

condition: value IS_EQUAL value
         | value IS_N_EQUAL value
         | value LESS value
         | value GREATER value
         | value LESS_EQ value
         | value GREATER_EQ value
;

value: num {
         expression_set_num($1);
     }
     | identifier
;

identifier: pidentifier {
              expression_set_var($1);
          }
          | pidentifier L_BRACE pidentifier R_BRACE {
              expression_set_var_arr_var($1, $3);
          }
          | pidentifier L_BRACE num R_BRACE {
              expression_set_var_arr_num($1, $3);
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
