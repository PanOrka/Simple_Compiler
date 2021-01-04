#include "i_graph.h"
#include "expr_checker.h"
#include "../parser_func/getters.h"
#include "generators/stack_generator.h"
#include "generators/val_generator.h"
#include "std_oper/std_oper.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_EXPR(expression_t *expr) {
    eval_check_0(expr);
    eval_check_1(expr);

    if (expr->type != expr_VALUE) {
        eval_check_2(expr);
    }

    add_to_list(expr, i_EXPR);
}

static void eval_expr_VALUE(expression_t const * const expr, FILE *file) {
    reg_set *r_set = get_reg_set();
    reg *assign_val = oper_get_assign_val_1(expr, file);
    uint8_t const assign_val_flags = (expr->mask & LEFT_SYM1_NUM) ? ASSIGN_VAL_IS_NUM : ASSIGN_VAL_NO_FLAGS;
    oper_set_assign_val_0(expr, assign_val, assign_val_flags, file);

    reg_m_drop_addr(r_set, TEMP_ADDR_1);
}

static void eval_expr_ADD(expression_t const * const expr, FILE *file) {
    reg_set *r_set = get_reg_set();
    
    if ((expr->mask & LEFT_SYM1_NUM) && (expr->mask & RIGHT_SYM1_NUM)) {
        num_t val = (num_t)expr->var_1[1].num + (num_t)expr->var_1[2].num;
        reg *assign_val = val_generate(val, file);
        oper_set_assign_val_0(expr, assign_val, ASSIGN_VAL_IS_NUM, file);
    } else {
        reg *assign_val_1 = oper_get_assign_val_1(expr, file);
        reg *assign_val_2 = oper_get_assign_val_2(expr, file);
        reg_m_promote(r_set, assign_val_1->addr);

        // if (!(expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) || (expr->mask & ASSIGN_SYM2_NUM)) {
        //     addr_t const eff_addr = expr->var_1[0].var->addr[0] + (addr_t)expr->var_2[0].num;

        // }
        fprintf(file, "ADD %c %c\n", assign_val_1->id, assign_val_2->id);
        oper_set_assign_val_0(expr, assign_val_1, ASSIGN_VAL_STASH, file);

        reg_m_drop_addr(r_set, TEMP_ADDR_1);
        reg_m_drop_addr(r_set, TEMP_ADDR_2);
    }
}

void eval_EXPR(i_graph **i_current, FILE *file) {
    expression_t const * const expr_curr = (*i_current)->payload;

    switch (expr_curr->type) {
        case expr_VALUE:
            eval_expr_VALUE(expr_curr, file);
            break;
        case expr_ADD:
            eval_expr_ADD(expr_curr, file);
            break;
        case expr_SUB:
            break;
        case expr_MUL:
            break;
        case expr_DIV:
            break;
        case expr_MOD:
            break;
        default:
            fprintf(stderr, "[EXPR]: Wrong type of expression!\n");
            exit(EXIT_FAILURE);
    }
}
