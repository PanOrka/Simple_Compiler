#ifndef EXPR_CHECKER_H
#define EXPR_CHECKER_H

#include "../parser_func/expressions.h"

/**
 * 
 * Check and evaluate 0 var of expr
 * 
*/
void eval_check_0(expression_t *expr);

/**
 * 
 * Check and evaluate 1 var of expr
 * 
*/
void eval_check_1(expression_t *expr);

/**
 * 
 * Check and evaluate 2 var of expr
 * 
*/
void eval_check_2(expression_t *expr);

#endif
