#ifndef STACK_GENERATOR_H
#define STACK_GENERATOR_H

#include "../../definitions.h"
#include <stdio.h>
#include <stdbool.h>

/**
 * 
 * Generate given address on stack_ptr
 *
*/
void stack_ptr_generate(addr_t addr);

/**
 * 
 * Clear stack_ptr
 * 
*/
void stack_ptr_clear();

/**
 * 
 * stack_ptr is initialized
 * 
*/
bool stack_ptr_initialized();

/**
 * 
 * stack_ptr is 0
 * 
*/
bool stack_ptr_is_null();

#endif
