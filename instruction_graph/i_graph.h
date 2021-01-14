#ifndef I_GRAPH_H
#define I_GRAPH_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    i_EXPR = 0,
    i_IF,
    i_ELSE,
    i_ENDIF,
    i_WHILE,
    i_ENDWHILE,
    i_REPEAT,
    i_UNTIL,
    i_FOR,
    i_ENDFOR,
    i_READ,
    i_WRITE
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

/**
 * 
 * Clearing useless IF-Else
 * 
*/
void i_graph_clear_if(bool cond, i_graph **i_current);

/**
 * 
 * Analyze if and store constant values, make them non-constant
 * 
*/
void i_graph_analyze_if(i_graph **i_current);

/**
 * 
 * Clearing useless While
 * 
*/
void i_graph_clear_while(bool cond, i_graph **i_current);

/**
 * 
 * Analyze while and store constant values, make them non-constant
 * 
*/
void i_graph_analyze_while(i_graph **i_current);

/**
 * 
 * Analyze for and store constant values, make them non-constant
 * 
*/
void i_graph_analyze_for(i_graph **i_current);

#endif
