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
reg_allocator oper_get_reg_for_variable(addr_t addr);

/**
 * 
 * Load variable to register
 * 
*/
void oper_load_variable_to_reg(reg *r, addr_t addr);

/**
 * 
 * Set stack pointer to address of array element from variable
 * 
*/
void oper_set_stack_ptr_addr_arr(addr_t var_idx_addr, addr_t arr_addr, uint64_t start_idx);

/**
 * 
 * r1 <= r2
 * Puts value of r2 to r1
 * 
*/
void oper_reg_swap(reg *r1, reg *r2);

/**
 * 
 * Store elements of array that were in registers
 * ARGUMENTS: [From, To) addresses of array
 * 
*/
void oper_store_array(const addr_t addr[2]);

/**
 * 
 * Drop all elements of array from registers
 * 
*/
void oper_drop_array(const addr_t addr[2]);

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

typedef struct {
    union {
        reg *reg;
        mpz_t constant;
    };
    bool is_reg;
} val;

/**
 * 
 * Flush all non before stored elements of constant array to memory
 * 
*/
void oper_flush_array_to_mem(symbol *arr);

/**
 * 
 * Operation on array_value list
 * 
 * RETURN: NULL or pointer to found arr_val
*/
array_value * oper_arr_val_find(array_value *arr_val, uint64_t idx_ui);

/**
 * 
 * Set assign_var with given ready register
 * 
 * ARGUMENTS: expression_t const * const expr <= from here we assign var[0]
 *            reg *assign_val <= register with value that we set assign_var
 *            uint8_t assign_val_flags <= information about assign_val
 * WARNING: New array non-constant elements will be mpz_init() by default
*/
void oper_set_assign_val_0(expression_t const * const expr,
                           val assign_val,
                           uint8_t assign_val_flags);

/**
 * 
 * Get assign_val_1 of expression to register
 * 
 * RETURN: Pointer to register with variable
 * WARNING: mpz_t constant in val will be initialized, please free this!!!!
*/
val oper_get_assign_val_1(expression_t const * const expr);

/**
 * 
 * Get assign_val_2 of expression to register
 * 
 * RETURN: Pointer to register with variable
 * WARNING: mpz_t constant in val will be initialized, please free this!!!!
*/
val oper_get_assign_val_2(expression_t const * const expr);

#endif
