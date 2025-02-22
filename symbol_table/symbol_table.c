#include "symbol_table.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

symbol_table symbol_table_create() {
    symbol_table s_table = { .v = vector_create(sizeof(symbol), alignof(symbol), SYMBOL_TABLE_INIT_SIZE) };

    return s_table;
}

int symbol_comparator_eq_id(void *a, void *b) {
    symbol const * const _a = (symbol *)a;
    symbol const * const _b = (symbol *)b;

    if (!(_a->identifier && _b->identifier)) {
        return 0;
    }

    return strcmp(_a->identifier, _b->identifier) == 0;
}

int symbol_comparator_eq_id_flags(void *a, void *b) {
    symbol const * const _a = (symbol *)a;
    symbol const * const _b = (symbol *)b;

    if (!(_a->identifier && _b->identifier)) {
        return 0;
    }

    return strcmp(_a->identifier, _b->identifier) == 0 && (_a->flags & _b->flags);
}

vector_element symbol_table_find_id(symbol_table *s_table, char *identifier, bool use_flags, uint8_t flags) {
    symbol to_comp = {
        .identifier = identifier,
        .flags = flags
    };

    vector_element element_found;
    if (use_flags) {
        element_found = VECTOR_FIND(s_table->v, to_comp, symbol_comparator_eq_id_flags);
    } else {
        element_found = VECTOR_FIND(s_table->v, to_comp, symbol_comparator_eq_id);
    }

    return element_found;
}

int symbol_comparator_eq_addr(void *a, void *b) {
    symbol const * const _a = (symbol *)a;
    symbol const * const _b = (symbol *)b;

    return _a->addr[0] <= _b->addr[0] && _b->addr[1] < _a->addr[1];
}

int symbol_comparator_eq_addr_flags(void *a, void *b) {
    symbol const * const _a = (symbol *)a;
    symbol const * const _b = (symbol *)b;

    return _a->addr[0] <= _b->addr[0] && _b->addr[1] < _a->addr[1] && (_a->flags & _b->flags);
}

vector_element symbol_table_find_addr(symbol_table *s_table, addr_t addr, bool use_flags, uint8_t flags) {
    symbol to_comp = {
        .addr = {addr, addr},
        .flags = flags
    };

    vector_element element_found;
    if (use_flags) {
        element_found = VECTOR_FIND(s_table->v, to_comp, symbol_comparator_eq_addr_flags);
    } else {
        element_found = VECTOR_FIND(s_table->v, to_comp, symbol_comparator_eq_addr);
    }

    return element_found;
}

idx_t symbol_table_add(symbol_table *s_table, const char *identifier, add_info _add_info, size_t size, uint8_t flags) {
    if (size <= 0) {
        fprintf(stderr, "[symbol_table]: ADD symbol got size <= 0, size = %lu!\n", size);
        exit(EXIT_FAILURE);
    }

    addr_t start_addr = (s_table->v.used_size > 0) ? ((symbol *)VECTOR_POP(s_table->v, NO_POP))->addr[1] : 0;

    symbol new_symbol = {
        .identifier = identifier,
        .addr = { start_addr, start_addr + size },
        ._add_info = _add_info,
        .flags = flags
    };

    if (flags & SYMBOL_IS_CONSTANT) {
        if (flags & SYMBOL_IS_ARRAY) {
            new_symbol.consts.arr_value = NULL;
        } else {
            mpz_init(new_symbol.consts.value);
            new_symbol.symbol_in_memory = true;
        }
    }

    VECTOR_ADD(s_table->v, new_symbol);

    return s_table->v.used_size - 1;
}

symbol symbol_table_pop(symbol_table *s_table) {
    return *((symbol *)VECTOR_POP(s_table->v, POP));
}

int64_t symbol_ptr_size(symbol *sym) {
    return sym->addr[1] - sym->addr[0];
}

symbol * symbol_table_find_by_idx(symbol_table *s_table, idx_t idx) {
    return vector_get(&(s_table->v), idx);
}
