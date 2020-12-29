#include "expr_checker.h"

static void symbol_not_initialized(expression_t *expr, char const *identifier) {
    fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", identifier);
    fprintf(stderr, "[EXPRESSION]:\n");
    print_expression(expr, stderr);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void index_out_of_bounds(expression_t *expr, int64_t num) {
    fprintf(stderr, "[I_GRAPH]: Index out of bounds: %ld!\n", num);
    fprintf(stderr, "[EXPRESSION]:\n");
    print_expression(expr, stderr);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void eval_check_0(expression_t *expr) {
    if (expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) {
        if (!(expr->mask & ASSIGN_SYM2_NUM)) {
            if (!(expr->var_2[0].var->flags & SYMBOL_INITIALIZED)) {
                symbol_not_initialized(expr, expr->var_2[0].var->identifier);
            } else if (expr->var_2[0].var->flags & SYMBOL_IS_ITER) {
                expr->var_2[0].addr = expr->var_2[0].var->addr[0];
                expr->addr_mask |= ASSIGN_SYM2_ADDR;
            }
        } else {
            expr->var_2[0].num -= expr->var_1[0].var->_add_info.start_idx;
            if (expr->var_2[0].num < 0 || expr->var_2[0].num >= symbol_ptr_size(expr->var_1[0].var)) {
                index_out_of_bounds(expr, expr->var_2[0].num);
            }
        }
    } else {
        if (expr->var_1[0].var->flags & SYMBOL_IS_ITER) {
            fprintf(stderr, "[EXPR]: Local iterators are non modifiable: %s!\n", expr->var_1[0].var->identifier);
            exit(EXIT_FAILURE);
        }

        expr->var_1[0].var->flags |= SYMBOL_INITIALIZED;
    }
}

void eval_check_1(expression_t *expr) {
    if (!(expr->mask & LEFT_SYM1_NUM)) {
        if (expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
            if (!(expr->mask & LEFT_SYM2_NUM)) {
                if (!(expr->var_2[1].var->flags & SYMBOL_INITIALIZED)) {
                    symbol_not_initialized(expr, expr->var_2[1].var->identifier);
                } else if (expr->var_2[1].var->flags & SYMBOL_IS_ITER) {
                    expr->var_2[1].addr = expr->var_2[1].var->addr[0];
                    expr->addr_mask |= LEFT_SYM2_ADDR;
                }
            } else {
                expr->var_2[1].num -= expr->var_1[1].var->_add_info.start_idx;
                if (expr->var_2[1].num < 0 || expr->var_2[1].num >= symbol_ptr_size(expr->var_1[1].var)) {
                    index_out_of_bounds(expr, expr->var_2[1].num);
                }
            }
        } else if (!(expr->var_1[1].var->flags & SYMBOL_INITIALIZED)) {
            symbol_not_initialized(expr, expr->var_1[1].var->identifier);
        } else if (expr->var_1[1].var->flags & SYMBOL_IS_ITER) {
            expr->var_1[1].addr = expr->var_1[1].var->addr[0];
            expr->addr_mask |= LEFT_SYM1_ADDR;
        }
    }
}

void eval_check_2(expression_t *expr) {
    if (!(expr->mask & RIGHT_SYM1_NUM)) {
        if (expr->var_1[2].var->flags & SYMBOL_IS_ARRAY) {
            if (!(expr->mask & RIGHT_SYM2_NUM)) {
                if (!(expr->var_2[2].var->flags & SYMBOL_INITIALIZED)) {
                    symbol_not_initialized(expr, expr->var_2[2].var->identifier);
                } else if (expr->var_2[2].var->flags & SYMBOL_IS_ITER) {
                    expr->var_2[2].addr = expr->var_2[2].var->addr[0];
                    expr->addr_mask |= RIGHT_SYM2_ADDR;
                }
            } else {
                expr->var_2[2].num -= expr->var_1[2].var->_add_info.start_idx;
                if (expr->var_2[2].num < 0 || expr->var_2[2].num >= symbol_ptr_size(expr->var_1[2].var)) {
                    index_out_of_bounds(expr, expr->var_2[2].num);
                }
            }
        } else if (!(expr->var_1[2].var->flags & SYMBOL_INITIALIZED)) {
            symbol_not_initialized(expr, expr->var_1[2].var->identifier);
        } else if (expr->var_1[2].var->flags & SYMBOL_IS_ITER) {
            expr->var_1[2].addr = expr->var_1[2].var->addr[0];
            expr->addr_mask |= RIGHT_SYM1_ADDR;
        }
    }
}
