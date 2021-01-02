#ifndef STACK_GENERATOR_H
#define STACK_GENERATOR_H

#include "../../definitions.h"
#include <stdio.h>

/**
 * 
 * Generate given address on stack_ptr
 *
*/
void stack_ptr_generate(addr_t addr, FILE *file);

/**
 * 
 * Clear stack_ptr
 * 
*/
void stack_ptr_clear();

#endif
