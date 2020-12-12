#include <stdio.h>
#include <stdlib.h>

#include "declarations.h"
#include "getters.h"

void declare(char *id) {
    symbol_table const * const s_table = get_symbol_table();
    symbol *s_find = symbol_table_find_id(s_table, id, false, SYMBOL_NO_FLAGS); 

    if (s_find != NULL) {
        fprintf(stderr, "[DECLARE]:Multiple declarations of one variable: %s!", id);
        exit(EXIT_FAILURE);
    }
    add_info a_info = {
        .start_idx = 0
    };
    symbol_table_add(s_table, id, a_info, 1, SYMBOL_NO_FLAGS);
}
