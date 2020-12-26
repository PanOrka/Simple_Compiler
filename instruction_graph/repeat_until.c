#include "../parser_func/expressions.h"
#include "i_level.h"
#include "expr_checker.h"

void add_REPEAT(); {
    i_level_add(i_REPEAT);
    add_to_list(NULL, i_REPEAT);
}

void add_UNTIL(expression_t *expr) {
    eval_check_1(expr);
    eval_check_2(expr);

    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_POP);
        if (b_type == i_REPEAT) {
            add_to_list(NULL, i_REPEAT);
            return;
        }
    }

    fprintf(stderr, "[REPEAT_UNTIL]: There is no matching REPEAT for UNTIL!\n");
    exit(EXIT_FAILURE);
}
