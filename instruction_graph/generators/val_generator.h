#ifndef VAL_GENERATOR_H
#define VAL_GENERATOR_H

#include "../../definitions.h"
#include "../std_oper/std_oper.h"

/**
 * 
 * Generate given num_t target value on value generator
 *
 * RETURN: Pointer to register of value generator register
*/
reg * val_generate(uint64_t target_value);

/**
 * 
 * Same as above but from mpz
 * 
*/
reg * val_generate_from_mpz(mpz_t target_value);

#endif
