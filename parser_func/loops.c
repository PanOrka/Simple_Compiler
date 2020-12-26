#include "loops.h"

#include "getters.h"

void loop_get(char const *iter, for_loop_t *loop) {
    if (!loop) {
        fprintf(stderr, "[LOOP]: NULLPTR!\n");
        exit(EXIT_FAILURE);
    }

    symbol_table *s_table = get_symbol_table();
    symbol *hide = symbol_table_find_id(s_table, iter, true, SYMBOL_NO_HIDDEN);

    hide->flags &= ~SYMBOL_NO_HIDDEN;
    loop->iterator.var = symbol_table_add(s_table, iter, (add_info){ .hide = hide }, 1, SYMBOL_IS_ITER | SYMBOL_INITIALIZED);

    expression_get(&(loop->range_vars));
    loop->range[0].var = symbol_table_add(s_table, NULL, (add_info){ .start_idx = 0 }, 1, SYMBOL_NO_FLAGS);
    loop->range[1].var = symbol_table_add(s_table, NULL, (add_info){ .start_idx = 0 }, 1, SYMBOL_NO_FLAGS);
}
