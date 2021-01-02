#ifndef VAL_GENERATOR_H
#define VAL_GENERATOR_H

#include "../../definitions.h"
#include "../../register_machine/reg_m.h"
#include <stdio.h>

/**
 * 
 * Generate given num_t target value on value generator
 *
 * RETURN: Pointer to register of value generator register
*/
reg * val_generate(num_t target_value, FILE *file);

#endif
