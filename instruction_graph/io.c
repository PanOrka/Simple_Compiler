#include "expr_checker.h"
#include "i_graph.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_READ(expression_t *expr) {
    eval_check_0(expr);
    add_to_list(expr, i_READ);
}

void eval_READ(i_graph **i_current, FILE *file) {

}

void add_WRITE(expression_t *expr) {
    eval_check_1(expr);
    add_to_list(expr, i_WRITE);
}

void eval_WRITE(i_graph **i_current, FILE *file) {
    
}
