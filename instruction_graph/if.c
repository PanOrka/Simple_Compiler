#include "../parser_func/expressions.h"
#include "i_level.h"
#include "expr_checker.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_IF(expression_t *expr) {
    eval_check_1(expr);
    eval_check_2(expr);

    i_level_add(i_IF);
    add_to_list(expr, i_IF);
}

void add_ELSE() {
    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_NOPOP);
        if (b_type == i_IF) {
            add_to_list(NULL, i_ELSE);
            return;
        }
    }

    fprintf(stderr, "[IF]: There is no matching IF for ELSE!\n");
    exit(EXIT_FAILURE);
}

void add_ENDIF() {
    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_POP);
        if (b_type == i_IF) {
            add_to_list(NULL, i_ENDIF);
            return;
        }
    }

    fprintf(stderr, "[IF]: There is no matching IF for ENDIF!\n");
    exit(EXIT_FAILURE);
}
