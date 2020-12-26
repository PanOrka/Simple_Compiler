%{
#include <stdio.h>
#include <stdint.h>

int yylex();
extern FILE *yyin;
int yyerror(char*);

extern int yylineno;

static FILE *compiler_out;

#include "parser_func/declarations.h"
#include "parser_func/expressions.h"
#include "parser_func/loops.h"
#include "instruction_graph/i_graph.h"
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
%left TO
%left DOWNTO
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
       expression_t *expr = malloc(sizeof(expression_t));
       expression_get(expr);

       // for debug purpose
       print_expression(expr, stdout);
       fprintf(stdout, ";\n");
       ////////////////////

       i_graph_add_instruction(expr, i_EXPR);
   }
   | IF condition THEN {
       expression_t *expr = malloc(sizeof(expression_t));
       expression_get(expr);

       // for debug purpose
       fprintf(stdout, "IF ");
       print_expression(expr, stdout);
       fprintf(stdout, " THEN\n");
       ////////////////////

       i_graph_add_instruction(expr, i_IF);
   }
   | ELSE {
       // for debug purpose
       fprintf(stdout, "ELSE\n");
       ////////////////////

       i_graph_add_instruction(NULL, i_ELSE);
   }
   | ENDIF {
       // for debug purpose
       fprintf(stdout, "ENDIF\n");
       ////////////////////

       i_graph_add_instruction(NULL, i_ENDIF);
   }
   | WHILE condition DO {
       expression_t *expr = malloc(sizeof(expression_t));
       expression_get(expr);

       // for debug purpose
       fprintf(stdout, "WHILE ");
       print_expression(expr, stdout);
       fprintf(stdout, " DO\n");
       ////////////////////

       i_graph_add_instruction(expr, i_WHILE);
   }
   | ENDWHILE {
       // for debug purpose
       fprintf(stdout, "ENDWHILE\n");
       ////////////////////

       i_graph_add_instruction(NULL, i_ENDWHILE);
   }
   | REPEAT {
       // for debug purpose
       fprintf(stdout, "REPEAT\n");
       ////////////////////

       i_graph_add_instruction(NULL, i_REPEAT);
   }
   | UNTIL condition END_EXPR {
       expression_t *expr = malloc(sizeof(expression_t));
       expression_get(expr);

       // for debug purpose
       fprintf(stdout, "UNTIL ");
       print_expression(expr, stdout);
       fprintf(stdout, ";\n");
       ////////////////////

       i_graph_add_instruction(expr, i_UNTIL);
   }
   | FOR pidentifier FROM value TO value DO {
       for_loop_t *loop = malloc(sizeof(for_loop_t));
       loop_get($2, loop);
       loop->type = loop_TO;

       // for debug purpose
       fprintf(stdout, "FOR %s ", $2);
       print_expression(&(loop->range_vars), stdout);
       fprintf(stdout, " DO (TO LOOP)\n");
       ////////////////////

       i_graph_add_instruction(loop, i_FOR);
   }
   | FOR pidentifier FROM value DOWNTO value DO {
       for_loop_t *loop = malloc(sizeof(for_loop_t));
       loop_get($2, loop);
       loop->type = loop_DOWNTO;

       // for debug purpose
       fprintf(stdout, "FOR %s ", $2);
       print_expression(&(loop->range_vars), stdout);
       fprintf(stdout, " DO (DOWNTO LOOP)\n");
       ////////////////////

       i_graph_add_instruction(loop, i_FOR);
   }
   | ENDFOR {
       // for debug purpose
       fprintf(stdout, "ENDFOR\n");
       ////////////////////

       i_graph_add_instruction(NULL, i_ENDFOR);
   }
   | READ identifier END_EXPR
   | WRITE value END_EXPR
   | END {
       i_graph_execute(compiler_out);
   }
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

condition: value IS_EQUAL value {
             expression_set_type((expr_type)cond_IS_EQUAL);
         }
         | value IS_N_EQUAL value {
             expression_set_type((expr_type)cond_IS_N_EQUAL);
         }
         | value LESS value {
             expression_set_type((expr_type)cond_LESS);
         }
         | value GREATER value {
             expression_set_type((expr_type)cond_GREATER);
         }
         | value LESS_EQ value {
             expression_set_type((expr_type)cond_LESS_EQ);
         }
         | value GREATER_EQ value {
             expression_set_type((expr_type)cond_GREATER_EQ);
         }
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
    rewind(yyin);
    int line_ctr = 1;
    char c;
    while (line_ctr != yylineno && (c = fgetc(yyin)) != EOF) {
        if (c == '\n') {
            ++line_ctr;
        }
    }

    if (line_ctr == yylineno) {
        fprintf(stderr, "Error in line: %d!\n", line_ctr);
        while ((c = fgetc(yyin)) != '\n') {
            fprintf(stderr, "\033[0;32m%c", c);
        }
        fprintf(stderr, "\n\033[0m");
    } else {
        fprintf(stderr, "[PARSER]: Something went wrong, line_ctr = %d!\n", line_ctr);
    }

    return 0;
}

void parse(FILE *in, FILE *out) {
    yyin = in;
    compiler_out = out;
    yyparse();
}
