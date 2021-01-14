#include "expr_checker.h"
#include "../parser_func/getters.h"

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
    symbol_table *s_table = get_symbol_table();
    symbol *var_1 = symbol_table_find_by_idx(s_table, expr->var_1[0].sym_idx);

    if (var_1->flags & SYMBOL_IS_ARRAY) {
        if (!(expr->mask & ASSIGN_SYM2_NUM)) {
            symbol *var_2 = symbol_table_find_by_idx(s_table, expr->var_2[0].sym_idx);
            if (!(var_2->flags & SYMBOL_INITIALIZED)) {
                symbol_not_initialized(expr, var_2->identifier);
            } else if (var_2->flags & SYMBOL_IS_ITER) {
                expr->var_2[0].addr = var_2->addr[0];
                expr->addr_mask |= ASSIGN_SYM2_ADDR;
            }
        } else {
            expr->var_2[0].num -= var_1->_add_info.start_idx;
            if (expr->var_2[0].num < 0 || expr->var_2[0].num >= symbol_ptr_size(var_1)) {
                index_out_of_bounds(expr, expr->var_2[0].num);
            }
        }
    } else {
        if (var_1->flags & SYMBOL_IS_ITER) {
            fprintf(stderr, "[EXPR]: Local iterators are non modifiable: %s!\n", var_1->identifier);
            exit(EXIT_FAILURE);
        }

        var_1->flags |= SYMBOL_INITIALIZED;
    }
}

void eval_check_1(expression_t *expr) {
    if (!(expr->mask & LEFT_SYM1_NUM)) {
        symbol_table *s_table = get_symbol_table();
        symbol *var_1 = symbol_table_find_by_idx(s_table, expr->var_1[1].sym_idx);

        if (var_1->flags & SYMBOL_IS_ARRAY) {
            if (!(expr->mask & LEFT_SYM2_NUM)) {
                symbol *var_2 = symbol_table_find_by_idx(s_table, expr->var_2[1].sym_idx);
                if (!(var_2->flags & SYMBOL_INITIALIZED)) {
                    symbol_not_initialized(expr, var_2->identifier);
                } else if (var_2->flags & SYMBOL_IS_ITER) {
                    expr->var_2[1].addr = var_2->addr[0];
                    expr->addr_mask |= LEFT_SYM2_ADDR;
                }
            } else {
                expr->var_2[1].num -= var_1->_add_info.start_idx;
                if (expr->var_2[1].num < 0 || expr->var_2[1].num >= symbol_ptr_size(var_1)) {
                    index_out_of_bounds(expr, expr->var_2[1].num);
                }
            }
        } else if (!(var_1->flags & SYMBOL_INITIALIZED)) {
            symbol_not_initialized(expr, var_1->identifier);
        } else if (var_1->flags & SYMBOL_IS_ITER) {
            expr->var_1[1].addr = var_1->addr[0];
            expr->addr_mask |= LEFT_SYM1_ADDR;
        }
    }
}

void eval_check_2(expression_t *expr) {
    if (!(expr->mask & RIGHT_SYM1_NUM)) {
        symbol_table *s_table = get_symbol_table();
        symbol *var_1 = symbol_table_find_by_idx(s_table, expr->var_1[2].sym_idx);

        if (var_1->flags & SYMBOL_IS_ARRAY) {
            if (!(expr->mask & RIGHT_SYM2_NUM)) {
                symbol *var_2 = symbol_table_find_by_idx(s_table, expr->var_2[2].sym_idx);
                if (!(var_2->flags & SYMBOL_INITIALIZED)) {
                    symbol_not_initialized(expr, var_2->identifier);
                } else if (var_2->flags & SYMBOL_IS_ITER) {
                    expr->var_2[2].addr = var_2->addr[0];
                    expr->addr_mask |= RIGHT_SYM2_ADDR;
                }
            } else {
                expr->var_2[2].num -= var_1->_add_info.start_idx;
                if (expr->var_2[2].num < 0 || expr->var_2[2].num >= symbol_ptr_size(var_1)) {
                    index_out_of_bounds(expr, expr->var_2[2].num);
                }
            }
        } else if (!(var_1->flags & SYMBOL_INITIALIZED)) {
            symbol_not_initialized(expr, var_1->identifier);
        } else if (var_1->flags & SYMBOL_IS_ITER) {
            expr->var_1[2].addr = var_1->addr[0];
            expr->addr_mask |= RIGHT_SYM1_ADDR;
        }
    }
}
