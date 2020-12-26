#ifndef LOOPS_H
#define LOOPS_H

#include "expressions.h"
#include "../definitions.h"
#include "../symbol_table/symbol_table.h"

typedef enum {
    TO = 0,
    DOWNTO
} loop_type;

typedef union {
    addr_t addr;
    symbol *var;
} var_t;

typedef struct {
    var_t iterator;
    var_t range[2]; // FROM, TO
    expression_t range_vars;
    loop_type type;
} for_loop_t;

/**
 * 
 * Copies current loop to given place in memory
 * 
 * ARGUMENTS: char * - iterator, for_loop_t * - allocated place for loop
*/
void loop_get(char const *iter, for_loop_t *loop);

#endif
