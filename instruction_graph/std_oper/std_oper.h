#ifndef STD_OPER_H
#define STD_OPER_H

#include "../../register_machine/reg_m.h"
#include <stdio.h>

/**
 * 
 * Get ready to use register
 *
 * RETURN: reg_allocator with ready to use register
*/
reg_allocator oper_get_reg_for_variable(addr_t addr, FILE *file);

/**
 * 
 * Load variable to register
 * 
*/
void oper_load_variable_to_reg(reg *r, addr_t addr, FILE *file);

/**
 * 
 * Set stack pointer to address of array element from variable
 * 
*/
void oper_set_stack_ptr_addr_arr(addr_t var_idx_addr, addr_t arr_addr, uint64_t start_idx, FILE *file);

#endif
