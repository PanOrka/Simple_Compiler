#include "expressions.h"
#include "getters.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static expression_t current;

void expression_set_var_arr_var(char *symbol_1, char *symbol_2) {
    if (current.spin >= 3) {
        fprintf(stderr, "[EXPRESSIONS]: Wrong spin value! assign_id: %s!\n", symbol_1);
        exit(EXIT_FAILURE);
    }
    symbol_table *s_table = get_symbol_table();

    if (!(current.var_1[current.spin].var = symbol_table_find_id(s_table, symbol_1, false, SYMBOL_NO_FLAGS))) {
        fprintf(stderr, "[EXPRESSIONS]: Symbol used but not declared, id: %s!\n", symbol_1);
        exit(EXIT_FAILURE);
    }

    if (!(current.var_2[current.spin].var = symbol_table_find_id(s_table, symbol_2, false, SYMBOL_NO_FLAGS))) {
        fprintf(stderr, "[EXPRESSIONS]: Symbol used but not declared, id: %s!\n", symbol_2);
        exit(EXIT_FAILURE);
    }

    ++(current.spin);
    free(symbol_1);
    free(symbol_2);
}

void expression_set_var_arr_num(char *symbol_1, int64_t symbol_2) {
    if (!(current.var_1[current.spin].var = symbol_table_find_id(get_symbol_table(), symbol_1, false, SYMBOL_NO_FLAGS))) {
        fprintf(stderr, "[EXPRESSIONS]: Symbol used but not declared, id: %s!\n", symbol_1);
        exit(EXIT_FAILURE);
    }
    current.var_2[current.spin].num = symbol_2;

    switch (current.spin) {
        case 0:
            current.mask |= ASSIGN_SYM2_NUM;
            break;
        case 1:
            current.mask |= LEFT_SYM2_NUM;
            break;
        case 2:
            current.mask |= RIGHT_SYM2_NUM;
            break;
        default:
            fprintf(stderr, "[EXPRESSIONS]: Wrong spin value! assign_id: %s!\n", symbol_1);
            exit(EXIT_FAILURE);
    }

    ++(current.spin);
    free(symbol_1);
}

void expression_set_var(char *symbol_1) {
    if (current.spin >= 3) {
        fprintf(stderr, "[EXPRESSIONS]: Wrong spin value! assign_id: %s!\n", symbol_1);
        exit(EXIT_FAILURE);
    }

    if (!(current.var_1[current.spin].var = symbol_table_find_id(get_symbol_table(), symbol_1, false, SYMBOL_NO_FLAGS))) {
        fprintf(stderr, "[EXPRESSIONS]: Symbol used but not declared, id: %s!\n", symbol_1);
        exit(EXIT_FAILURE);
    }

    ++(current.spin);
    free(symbol_1);
}

void expression_set_num(int64_t symbol_1) {
    current.var_1[current.spin].num = symbol_1;

    switch (current.spin) {
        case 0:
            fprintf(stderr, "[EXPRESSIONS]: Got 0 spin on int64_t symbol_1!\n");
            exit(EXIT_FAILURE);
        case 1:
            current.mask |= LEFT_SYM1_NUM;
            break;
        case 2:
            current.mask |= RIGHT_SYM1_NUM;
            break;
        default:
            fprintf(stderr, "[EXPRESSIONS]: Wrong spin value!\n");
            exit(EXIT_FAILURE);
    }

    ++(current.spin);
}

void expression_set_type(expr_type type) {
    current.type = type;
}

expression_t * expression_get() {
    expression_t *temp = malloc(sizeof(expression_t));
    if (!temp) {
        fprintf(stderr, "[EXPRESSIONS]: Couldn't allocate memory for expression!\n");
        exit(EXIT_FAILURE);
    }
    memcpy(temp, &current, sizeof(expression_t));
    memset(&current, '\0', sizeof(expression_t));

    return temp;
}

static char print_arr[] = {'v', '+', '-', '*', '/', '%'};

void print_expression(expression_t *expr) {
    printf("%s", expr->var_1[0].var->identifier);
    if (expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) {
        printf("(");
        if (expr->mask & ASSIGN_SYM2_NUM) {
            printf("%ld)", expr->var_2[0].num);
        } else {
            printf("%s)", expr->var_2[0].var->identifier);
        }
    }
    printf(" := ");

    if (expr->mask & LEFT_SYM1_NUM) {
        printf("%ld", expr->var_1[1].num);
    } else {
        printf("%s", expr->var_1[1].var->identifier);
        if (expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
            printf("(");
            if (expr->mask & LEFT_SYM2_NUM) {
                printf("%ld)", expr->var_2[1].num);
            } else {
                printf("%s)", expr->var_2[1].var->identifier);
            }
        }
    }

    if (expr->type != expr_VALUE) {
        printf(" %c ", print_arr[expr->type]);
        if (expr->mask & RIGHT_SYM1_NUM) {
            printf("%ld", expr->var_1[2].num);
        } else {
            printf("%s", expr->var_1[2].var->identifier);
            if (expr->var_1[2].var->flags & SYMBOL_IS_ARRAY) {
                printf("(");
                if (expr->mask & RIGHT_SYM2_NUM) {
                    printf("%ld)", expr->var_2[2].num);
                } else {
                    printf("%s)", expr->var_2[2].var->identifier);
                }
            }
        }
    }

    printf("\n");
}
