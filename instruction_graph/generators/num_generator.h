#ifndef NUM_GENERATOR_H
#define NUM_GENERATOR_H

#include "../../parser_func/getters.h"
#include "../../definitions.h"

#include <gmp.h>
#include <stdio.h>

/**
 * 
 * Generate value on given register
 * 
*/
void generate_value(reg *r, mpz_t curr_val, mpz_t target_val, bool reset);

#endif
