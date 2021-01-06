#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../definitions.h"
#include "../vector/vector.h"

#include <gmp.h>

#ifndef SYMBOL_TABLE_INIT_SIZE
#define SYMBOL_TABLE_INIT_SIZE 32
#endif

#ifndef SYMBOL_NO_FLAGS
#define SYMBOL_NO_FLAGS 0b00000000
#endif

#ifndef SYMBOL_NO_HIDDEN
#define SYMBOL_NO_HIDDEN 0b00000001
#endif

#ifndef SYMBOL_INITIALIZED
#define SYMBOL_INITIALIZED 0b00000010
#endif

#ifndef SYMBOL_IS_ARRAY
#define SYMBOL_IS_ARRAY 0b00000100
#endif

#ifndef SYMBOL_IS_ITER
#define SYMBOL_IS_ITER 0b00001000
#endif

#ifndef SYMBOL_IS_CONSTANT
#define SYMBOL_IS_CONSTANT 0b00010000
#endif

typedef struct symbol symbol;

typedef union {
    uint64_t start_idx;
    symbol *hide;
} add_info;

typedef struct array_value array_value;

struct array_value {
    array_value *next;
    mpz_t value;
    uint64_t n;
    bool is_in_memory;
};

typedef union {
    mpz_t value;
    array_value *arr_value;
} const_info;

struct symbol {
    const char *identifier;
    const addr_t addr[2]; // address is [FROM, TO), cuz TO - FROM = size
    const add_info _add_info; // TODO: union with address of PRZYKRYTA zmienna if it's iterator, to change it on POP() start_idx is uint64_t so w/e about memory
    const_info consts;
    uint8_t flags;
};

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
 * Find symbol in table by id & flags (optional)
 * Flags are just simple intersection - find first intersection > 0
 * 
 * ARGUMENTS: Pointer to symbol_table, identifier of symbol to search for, bool use_flags, flags
 * RETURN: Pointer to found symbol
*/
symbol * symbol_table_find_id(symbol_table *s_table, char *identifier, bool use_flags, uint8_t flags);

/**
 * 
 * Find symbol in table by address & flags (optional)
 * Flags are just simple intersection - find first intersection > 0
 * 
 * ARGUMENTS: Pointer to symbol_table, address of symbol to search for, bool use_flags, flags
 * RETURN: Pointer to found symbol
*/
symbol * symbol_table_find_addr(symbol_table *s_table, addr_t addr, bool use_flags, uint8_t flags);

/**
 * 
 * Add new symbol to symbol_table
 * 
 * RETURN: Pointer to added symbol
*/
symbol * symbol_table_add(symbol_table *s_table, const char *identifier, add_info _add_info, size_t size, uint8_t flags);

/**
 * 
 * Pop symbol from symbol_table
 * 
 * RETURN: COPY of popped symbol
*/
symbol symbol_table_pop(symbol_table *s_table);

/**
 * 
 * Size of symbol
 * 
 * RETURN: Size of symbol
*/
int64_t symbol_ptr_size(symbol *sym);

#endif
