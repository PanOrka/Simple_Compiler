#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "../definitions.h"
#include "../symbol_table/symbol_table.h"

#include <stdio.h>

#ifndef LEFT_SYM1_NUM
#define LEFT_SYM1_NUM 0b10000000
#endif

#ifndef LEFT_SYM2_NUM
#define LEFT_SYM2_NUM 0b01000000
#endif

#ifndef RIGHT_SYM1_NUM
#define RIGHT_SYM1_NUM 0b00100000
#endif

#ifndef RIGHT_SYM2_NUM
#define RIGHT_SYM2_NUM 0b00010000
#endif

#ifndef ASSIGN_SYM2_NUM
#define ASSIGN_SYM2_NUM 0b00000001
#endif

#ifndef LEFT_SYM1_ADDR
#define LEFT_SYM1_ADDR 0b10000000
#endif

#ifndef LEFT_SYM2_ADDR
#define LEFT_SYM2_ADDR 0b01000000
#endif

#ifndef RIGHT_SYM1_ADDR
#define RIGHT_SYM1_ADDR 0b00100000
#endif

#ifndef RIGHT_SYM2_ADDR
#define RIGHT_SYM2_ADDR 0b00010000
#endif

#ifndef ASSIGN_SYM2_ADDR
#define ASSIGN_SYM2_ADDR 0b00000001
#endif

#ifndef EXPR_NONE
#define EXPR_NONE 0b00000000
#endif

typedef enum {
    expr_VALUE = 0,
    expr_ADD,
    expr_SUB,
    expr_MUL,
    expr_DIV,
    expr_MOD,
    cond_IS_EQUAL,
    cond_IS_N_EQUAL,
    cond_LESS,
    cond_GREATER,
    cond_LESS_EQ,
    cond_GREATER_EQ,
    loop_FOR, // this is just for correct print, we don't need it rly
    expr_READ, // same as above
    expr_WRITE
} expr_type;

typedef union {
    addr_t addr;
    int64_t num;
    idx_t sym_idx;
} value_t;

typedef struct {
    value_t var_1[3];
    value_t var_2[3];
    expr_type type;
    uint8_t mask;
    uint8_t spin;
    uint8_t addr_mask;
} expression_t;

/**
 * 
 * Setting expression element from assignment
 * 
*/
void expression_set_var_arr_var(char *symbol_1, char *symbol_2);

/**
 * 
 * Setting expression element from assignment
 * 
*/
void expression_set_var_arr_num(char *symbol_1, int64_t symbol_2);

/**
 * 
 * Setting expression element from assignment
 * 
*/
void expression_set_var(char *symbol_1);

/**
 * 
 * Setting expression element from assignment
 * 
*/
void expression_set_num(int64_t symbol_1);

/**
 * 
 * Setting type of expression
 * 
*/
void expression_set_type(expr_type type);

/**
 * 
 * Reduces spin of current expression by one
 * 
*/
void expression_spin_reduce();

/**
 * 
 * Get expression
 * 
 * RETURN: Copy current expression to given pointer
*/
void expression_get(expression_t *expr);

/**
 * 
 * print expression
 * 
*/
void print_expression(expression_t *expr, FILE *file);

#endif
