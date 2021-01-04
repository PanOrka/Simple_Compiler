#include "../parser_func/loops.h"
#include "i_level.h"
#include "expr_checker.h"
#include "../parser_func/getters.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_FOR(for_loop_t *loop) {
    eval_check_1(&(loop->range_vars));
    eval_check_2(&(loop->range_vars));

    i_level_add(i_FOR);
    add_to_list(loop, i_FOR);
}

void eval_FOR(i_graph **i_current) {
    
}

void add_ENDFOR() {
    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_POP);
        if (b_type == i_FOR) {
            add_to_list(NULL, i_ENDFOR);

            symbol_table *s_table = get_symbol_table();
            int32_t ctr = 0;
            while (true) {
                symbol popped = symbol_table_pop(s_table);
                free((void *)popped.identifier);
                ++ctr;

                if (popped.flags & SYMBOL_IS_ITER) {
                    if (popped._add_info.hide) {
                        popped._add_info.hide->flags |= SYMBOL_NO_HIDDEN;
                    }

                    if (ctr != 2) {
                        fprintf(stderr, "[FOR]: Incorrect value of popped values counter: %d!\n", ctr);
                        exit(EXIT_FAILURE);
                    }

                    break;
                }

                if (ctr > 2) {
                    fprintf(stderr, "[FOR]: Counter of popped values > 2!\n");
                    exit(EXIT_FAILURE);
                }
            }
            return;
        }
    }

    fprintf(stderr, "[FOR]: There is no matching FOR for ENDFOR!\n");
    exit(EXIT_FAILURE);
}

void eval_ENDFOR(i_graph **i_current) {
    
}
