#include "loops.h"

#include "getters.h"

void loop_get(char *iter, for_loop_t *loop) {
    if (!loop) {
        fprintf(stderr, "[LOOP]: NULLPTR!\n");
        exit(EXIT_FAILURE);
    }

    symbol_table *s_table = get_symbol_table();
    vector_element hide = symbol_table_find_id(s_table, iter, true, SYMBOL_NO_HIDDEN);

    if (hide.element_ptr) {
        ((symbol *)hide.element_ptr)->flags &= ~SYMBOL_NO_HIDDEN;
    }
    const idx_t iter_idx = symbol_table_add(s_table, iter, (add_info){ .hide_idx = hide.idx }, 1, SYMBOL_IS_ITER | SYMBOL_INITIALIZED | SYMBOL_NO_HIDDEN);
    loop->iterator = symbol_table_find_by_idx(s_table, iter_idx)->addr[0];

    expression_get(&(loop->range_vars));
    loop->range_vars.type = loop_FOR; // for debug

    const idx_t range_idx = symbol_table_add(s_table, NULL, (add_info){ .start_idx = 0 }, 1, SYMBOL_NO_FLAGS);
    loop->range = symbol_table_find_by_idx(s_table, range_idx)->addr[0];
}
