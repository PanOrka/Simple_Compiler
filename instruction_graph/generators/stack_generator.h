#ifndef STACK_GENERATOR_H
#define STACK_GENERATOR_H

#include "../../parser_func/getters.h"
#include "../../definitions.h"
#include <stdio.h>

/**
 * 
 * Generate given address on stack_ptr
 *
 * RETURN: Register of stack_ptr 
*/
reg * stack_ptr_generate(addr_t addr, FILE *file);

#endif
