#ifndef LOOPS_H
#define LOOPS_H

#include "expressions.h"
#include "../definitions.h"
#include "../symbol_table/symbol_table.h"

typedef enum {
    loop_TO = 0,
    loop_DOWNTO
} loop_type;

typedef struct {
    addr_t iterator;
    addr_t range; // TO
    expression_t range_vars;
    loop_type type;
} for_loop_t;

/**
 * 
 * Copies current loop to given place in memory
 * 
 * ARGUMENTS: char * - iterator, for_loop_t * - allocated place for loop
*/
void loop_get(char *iter, for_loop_t *loop);

#endif
