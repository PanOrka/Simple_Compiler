#ifndef NUM_GENERATOR_H
#define NUM_GENERATOR_H

#include "../../parser_func/getters.h"
#include "../../definitions.h"
#include <stdio.h>

/**
 * 
 * Generate value on given register
 * 
*/
void generate_value(reg *r, addr_t curr_val, addr_t target_val, FILE *file, bool reset);

#endif
