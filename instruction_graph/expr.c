#include "i_graph.h"

#include "../parser_func/expressions.h"
#include "../definitions.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_EXPR(expression_t *expr) {
    if (expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) {
        if (!(expr->mask & ASSIGN_SYM2_NUM)) {
            if (!(expr->var_2[0].var->flags & SYMBOL_INITIALIZED)) {
                fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_2[0].var->identifier);
                fprintf(stderr, "[EXPRESSION]:\n");
                print_expression(expr, stderr);
                fprintf(stderr, "\n");
                exit(EXIT_FAILURE);
            } else if (expr->var_2[0].var->flags & SYMBOL_IS_ITER) {
                expr->var_2[0].addr = expr->var_2[0].var->addr[0];
                expr->addr_mask |= ASSIGN_SYM2_ADDR;
            }
        }
    } else {
        if (expr->var_1[0].var->flags & SYMBOL_IS_ITER) {
            fprintf(stderr, "[EXPR]: Local iterators are non modifiable: %s!\n", expr->var_1[0].var->identifier);
            exit(EXIT_FAILURE);
        }

        expr->var_1[0].var->flags |= SYMBOL_INITIALIZED;
    }

    if (!(expr->mask & LEFT_SYM1_NUM)) {
        if (expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
            if (!(expr->mask & LEFT_SYM2_NUM)) {
                if (!(expr->var_2[1].var->flags & SYMBOL_INITIALIZED)) {
                    fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_2[1].var->identifier);
                    fprintf(stderr, "[EXPRESSION]:\n");
                    print_expression(expr, stderr);
                    fprintf(stderr, "\n");
                    exit(EXIT_FAILURE);
                } else if (expr->var_2[1].var->flags & SYMBOL_IS_ITER) {
                    expr->var_2[1].addr = expr->var_2[1].var->addr[0];
                    expr->addr_mask |= LEFT_SYM2_ADDR;
                }
            }
        } else if (!(expr->var_1[1].var->flags & SYMBOL_INITIALIZED)) {
            fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_1[1].var->identifier);
            fprintf(stderr, "[EXPRESSION]:\n");
            print_expression(expr, stderr);
            fprintf(stderr, "\n");
            exit(EXIT_FAILURE);
        } else if (expr->var_1[1].var->flags & SYMBOL_IS_ITER) {
            expr->var_1[1].addr = expr->var_1[1].var->addr[0];
            expr->addr_mask |= LEFT_SYM1_ADDR;
        }
    }

    if (expr->type != expr_VALUE) {
        if (!(expr->mask & RIGHT_SYM1_NUM)) {
            if (expr->var_1[2].var->flags & SYMBOL_IS_ARRAY) {
                if (!(expr->mask & RIGHT_SYM2_NUM)) {
                    if (!(expr->var_2[2].var->flags & SYMBOL_INITIALIZED)) {
                        fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_2[2].var->identifier);
                        fprintf(stderr, "[EXPRESSION]:\n");
                        print_expression(expr, stderr);
                        fprintf(stderr, "\n");
                        exit(EXIT_FAILURE);
                    } else if (expr->var_2[2].var->flags & SYMBOL_IS_ITER) {
                        expr->var_2[2].addr = expr->var_2[2].var->addr[0];
                        expr->addr_mask |= RIGHT_SYM2_ADDR;
                    }
                }
            } else if (!(expr->var_1[2].var->flags & SYMBOL_INITIALIZED)) {
                fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_1[2].var->identifier);
                fprintf(stderr, "[EXPRESSION]:\n");
                print_expression(expr, stderr);
                fprintf(stderr, "\n");
                exit(EXIT_FAILURE);
            } else if (expr->var_1[2].var->flags & SYMBOL_IS_ITER) {
                expr->var_1[2].addr = expr->var_1[2].var->addr[0];
                expr->addr_mask |= RIGHT_SYM1_ADDR;
            }
        }
    }

    add_to_list(expr, i_EXPR);
}
