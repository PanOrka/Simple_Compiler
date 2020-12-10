#include "symbol_table.h"

#include <string.h>
#include <stdio.h>

symbol_table symbol_table_create() {
    symbol_table s_table = { .v = vector_create(sizeof(symbol), alignof(symbol), SYMBOL_TABLE_INIT_SIZE) };

    return s_table;
}

int symbol_comparator_eq(void *a, void *b) {
    symbol *_a = (symbol *)a;
    symbol *_b = (symbol *)b;

    return (strcmp(_a->identifier, _b->identifier) == 0) ? 1 : 0;
}

symbol * symbol_table_find(symbol_table *s_table, char *identifier) {
    symbol to_comp;
    to_comp.identifier = identifier;
    symbol *element_found = (symbol *)(VECTOR_FIND(s_table->v, to_comp, symbol_comparator_eq).element_ptr);

    return element_found;
}

void symbol_table_add(symbol_table *s_table, const char *identifier, uint64_t start_idx, size_t size, uint8_t flags) {
    if (size <= 0) {
        fprintf(stderr, "[symbol_table]: ADD symbol got size <= 0, size = %lu!\n");
        exit(EXIT_FAILURE);
    }

    addr_t start_addr = (s_table->v.used_size > 0) ? ((symbol *)VECTOR_POP(s_table->v, NO_POP))->addr[1] : 0;

    symbol new_symbol = {
        .identifier = identifier,
        .addr = { start_addr, start_addr + size },
        .start_idx = start_idx,
        .flags = flags
    };

    VECTOR_ADD(s_table->v, new_symbol);
}

symbol symbol_table_pop(symbol_table *s_table) {
    return *((symbol *)VECTOR_POP(s_table->v, POP));
}
