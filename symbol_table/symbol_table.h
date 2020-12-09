#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../definitions.h"
#include "../vector/vector.h"

#ifndef SYMBOL_TABLE_INIT_SIZE
#define SYMBOL_TABLE_INIT_SIZE 32
#endif

typedef struct {
    const char *identifier;
    const addr_t addr[2];
    const size_t start_idx;
    uint8_t flags;
} symbol;

typedef struct {
    vector v;
} symbol_table;

/**
 * 
 * Create new symbol_table
 * 
 * RETURN: Copy of created symbol_table
*/
symbol_table symbol_table_create();

/**
 * 
 * Find symbol in table
 * 
 * ARGUMENTS: Pointer to symbol_table and identifier of symbol to search for
 * RETURN: Pointer to found symbol
*/
symbol * symbol_table_find(symbol_table *s_table, char *identifier);

#endif
