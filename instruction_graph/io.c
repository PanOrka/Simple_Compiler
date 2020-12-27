#include "expr_checker.h"
#include "i_graph.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_READ(expression_t *expr) {
    eval_check_0(expr);
    add_to_list(expr, i_READ);
}

void add_WRITE(expression_t *expr) {
    eval_check_1(expr);
    add_to_list(expr, i_WRITE);
}
