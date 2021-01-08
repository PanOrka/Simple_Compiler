#ifndef COND_H
#define COND_H

#include "../../parser_func/expressions.h"

#include <stdbool.h>
#include <gmp.h>

/**
 * 
 * Return bool value of condition
 * 
*/
bool cond_val_from_const(mpz_t src_1, mpz_t src_2, expr_type type);

#endif
