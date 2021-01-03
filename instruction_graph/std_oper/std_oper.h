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
 * ARGUMENTS: [From, To) addresses of array,
 *            bool drop_all - flag whether to drop every reg with element of array
 * 
*/
void oper_store_array(const addr_t addr[2], bool drop_all, FILE *file);

/**
 * 
 * ASSIGNS
 * 
*/
#ifndef ASSIGN_VAL_NO_FLAGS
#define ASSIGN_VAL_NO_FLAGS 0b00000000
#endif

#ifndef ASSIGN_VAL_IS_NUM
#define ASSIGN_VAL_IS_NUM 0b00000001
#endif

#ifndef ASSIGN_VAL_STASH
#define ASSIGN_VAL_STASH 0b00000010
#endif

/**
 * 
 * Set assign_var with given ready register
 * 
 * ARGUMENTS: expression_t const * const expr <= from here we assign var[0]
 *            reg *assign_val <= register with value that we set assign_var
 *            uint8_t assign_val_flags <= information about assign_val
*/
void oper_set_assign_val_0(expression_t const * const expr,
                           reg *assign_val,
                           uint8_t assign_val_flags,
                           FILE *file);

/**
 * 
 * Get assign_val_1 of expression to register
 * 
 * RETURN: Pointer to register with variable
*/
reg * oper_get_assign_val_1(expression_t const * const expr, FILE *file);

/**
 * 
 * Get assign_val_2 of expression to register
 * 
 * RETURN: Pointer to register with variable
*/
reg * oper_get_assign_val_2(expression_t const * const expr, FILE *file);

#endif
