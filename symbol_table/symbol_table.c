#include "symbol_table.h"

#include <string.h>

symbol_table symbol_table_create() {
    symbol_table s_table = { .v = vector_create(sizeof(symbol), alignof(symbol), SYMBOL_TABLE_INIT_SIZE) };

    return s_table;
}

int symbol_comparator_eq(void *a, void *b) {
    symbol *_a = (symbol *)a;
    symbol *_b = (symbol *)b;

    if (strcmp(_a->identifier, _b->identifier) == 0) {
        return 1;
    }

    return 0;
}

symbol * symbol_table_find(symbol_table *s_table, char *identifier) {
    symbol to_comp;
    to_comp.identifier = identifier;
    symbol *element_found = (symbol *)(VECTOR_FIND(s_table->v, to_comp, symbol_comparator_eq).element_ptr);

    return element_found;
}
