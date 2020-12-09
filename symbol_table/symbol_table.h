#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../definitions.h"
#include "../vector/vector.h"

typedef struct {
    char *identifier;
    addr_t addr[2];
    size_t start_idx;
    
} symbol;

typedef struct {
    vector v;
} symbol_table;

#endif
