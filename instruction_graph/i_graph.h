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
 * Set asm_print file and check for correct amount of branches
 * 
*/
void i_graph_set_and_check(FILE *file);

/**
 * 
 * Execute instructions from instruction graph while start != to
 * 
*/
void i_graph_execute(i_graph *to);

/**
 * 
 * Frees i_graph, register set and symbol table
 * 
*/
void i_graph_free_all();

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
 * Finding instructions of this particular loop
 * 
*/
void i_graph_while_find(i_graph *i_while, i_graph **i_endwhile);

/**
 * 
 * Analyze while and store constant values, make them non-constant
 * 
*/
void i_graph_analyze_while(i_graph **i_current);

/**
 * 
 * Finding instructions of this particular loop
 * 
*/
void i_graph_for_find(i_graph *i_for, i_graph **i_endfor);

/**
 * 
 * Analyze for and store constant values, make them non-constant
 * 
*/
void i_graph_analyze_for(i_graph **i_current);

/**
 * 
 * Analyze repeat_until and store constant values, make them non-constant
 * 
*/
void i_graph_analyze_repeat_until(i_graph **i_current);

#endif
