#include <stdio.h>
#include <stdlib.h>

#include "declarations.h"
#include "getters.h"

void declare_var(char *id) {
    symbol_table *s_table = get_symbol_table();
    symbol *s_find = symbol_table_find_id(s_table, id, false, SYMBOL_NO_FLAGS); 

    if (s_find != NULL) {
        fprintf(stderr, "[DECLARE]:Multiple declarations of one variable: %s!\n", id);
        exit(EXIT_FAILURE);
    }
    add_info a_info = {
        .start_idx = 0
    };
    symbol_table_add(s_table, id, a_info, 1, SYMBOL_NO_FLAGS);
}

void declare_array(char *id, int64_t start_idx, int64_t end_idx) {
    if (start_idx > end_idx) {
        fprintf(stderr, "[DECLARE]: end_idx < start_idx: %s(%ld:%ld)!\n", id, start_idx, end_idx);
        exit(EXIT_FAILURE);
    }
    symbol_table *s_table = get_symbol_table();
    symbol *s_find = symbol_table_find_id(s_table, id, false, SYMBOL_NO_FLAGS);

    if (s_find != NULL) {
        fprintf(stderr, "[DECLARE]: Multiple declarations of one variable: %s!\n", id);
        exit(EXIT_FAILURE);
    }
    add_info a_info = {
        .start_idx = start_idx
    };
    symbol_table_add(s_table, id, a_info, end_idx - start_idx + 1, SYMBOL_NO_FLAGS);
}
