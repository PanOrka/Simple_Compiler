#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "../definitions.h"
#include "../symbol_table/symbol_table.h"

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

#ifndef EXPR_NONE
#define EXPR_NONE 0b00000000
#endif

typedef enum {
    VALUE = 0,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD
} expr_type;

typedef union {
    symbol *var;
    int64_t num;
} value_t;

typedef struct {
    value_t var_1[3];
    value_t var_2[3];
    expr_type type;
    uint8_t mask;
    uint8_t spin;
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
 * Get expression
 * 
 * RETURN: Current executed expression
*/
expression_t expression_get();

#endif
