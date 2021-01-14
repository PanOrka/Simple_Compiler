#ifndef COND_H
#define COND_H

#include "../../parser_func/expressions.h"
#include "../std_oper/std_oper.h"

#include <stdbool.h>
#include <gmp.h>

/**
 * 
 * Return bool value of condition
 * 
*/
bool cond_val_from_const(mpz_t src_1, mpz_t src_2, expr_type type);

/**
 * 
 * Return bool value in register
 * 
*/
reg * cond_val_from_vals(val src_1, val src_2, expr_type type);

#endif
