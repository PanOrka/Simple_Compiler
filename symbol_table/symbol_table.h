#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../definitions.h"
#include "../vector/vector.h"

#ifndef SYMBOL_TABLE_INIT_SIZE
#define SYMBOL_TABLE_INIT_SIZE 32
#endif

#ifndef NO_FLAGS
#define NO_FLAGS 0b00000000
#endif

#ifndef ITERATOR_FLAG
#define ITERATOR_FLAG 0b00000001
#endif

typedef union {
    uint64_t start_idx;
    symbol *hide;
} add_info;

typedef struct {
    const char *identifier;
    const addr_t addr[2]; // address is [FROM, TO), cuz TO - FROM = size
    const add_info _add_info; // TODO: union with address of PRZYKRYTA zmienna if it's iterator, to change it on POP() start_idx is uint64_t so w/e about memory
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

/**
 * 
 * Add new symbol to symbol_table
 * 
*/
void symbol_table_add(symbol_table *s_table, const char *identifier, uint64_t start_idx, size_t size, uint8_t flags);

/**
 * 
 * Pop symbol from symbol_table
 * 
 * RETURN: COPY of popped symbol
*/
symbol symbol_table_pop(symbol_table *s_table);

#endif
