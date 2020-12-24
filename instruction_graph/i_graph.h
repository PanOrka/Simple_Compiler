#ifndef I_GRAPH_H
#define I_GRAPH_H

#include <stdio.h>

typedef enum {
    EXPR = 0
} instruction_type;

typedef struct i_graph i_graph;

struct i_graph {
    void *payload;
    i_graph *prev;
    i_graph *next;
    instruction_type i_type;
};

/**
 * 
 * Adding instruction to instruction graph
 * 
*/
void i_graph_add_instruction(void *payload, instruction_type i_type);

/**
 * 
 * Execute instructions from instruction graph
 * 
*/
void i_graph_execute(FILE *file);

#endif
