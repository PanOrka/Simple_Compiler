#ifndef STD_OPER_H
#define STD_OPER_H

#include "../../register_machine/reg_m.h"
#include "../../parser_func/expressions.h"
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

/**
 * 
 * r1 <= r2
 * Puts value of r2 to r1
 * 
*/
void oper_reg_swap(reg *r1, reg *r2, FILE *file);

/**
 * 
 * Store elements of array that were in registers
 * ARGUMENTS: [From, To) addresses of array
 * 
*/
void oper_store_array(const addr_t addr[2], FILE *file);

/**
 * 
 * Get assign_val_1 of expression to register
 * 
 * RETURN: Pointer to register with variable
*/
reg * oper_get_assign_val_1(expression_t const * const expr, FILE *file);

#endif
