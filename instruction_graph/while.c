#include "../parser_func/expressions.h"
#include "i_level.h"
#include "expr_checker.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_WHILE(expression_t *expr) {
    eval_check_1(expr);
    eval_check_2(expr);

    i_level_add(i_WHILE);
    add_to_list(expr, i_WHILE);
}

void eval_WHILE(i_graph **i_current, FILE *file) {
    
}

void add_ENDWHILE() {
    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_POP);
        if (b_type == i_WHILE) {
            add_to_list(NULL, i_ENDWHILE);
            return;
        }
    }

    fprintf(stderr, "[WHILE]: There is no matching WHILE for ENDWHILE!\n");
    exit(EXIT_FAILURE);
}

void eval_ENDWHILE(i_graph **i_current, FILE *file) {
    
}
