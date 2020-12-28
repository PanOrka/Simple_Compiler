#include "loops.h"

#include "getters.h"

void loop_get(char *iter, for_loop_t *loop) {
    if (!loop) {
        fprintf(stderr, "[LOOP]: NULLPTR!\n");
        exit(EXIT_FAILURE);
    }

    symbol_table *s_table = get_symbol_table();
    symbol *hide = symbol_table_find_id(s_table, iter, true, SYMBOL_NO_HIDDEN);

    if (hide) {
        hide->flags &= ~SYMBOL_NO_HIDDEN;
    }
    loop->iterator.var = symbol_table_add(s_table, iter, (add_info){ .hide = hide }, 1, SYMBOL_IS_ITER | SYMBOL_INITIALIZED | SYMBOL_NO_HIDDEN);

    expression_get(&(loop->range_vars));
    loop->range_vars.type = loop_FOR; // for debug

    loop->range.var = symbol_table_add(s_table, NULL, (add_info){ .start_idx = 0 }, 1, SYMBOL_NO_FLAGS);
}
