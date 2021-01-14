#include "../parser_func/expressions.h"
#include "i_level.h"
#include "expr_checker.h"
#include "std_oper/std_oper.h"
#include "conditions/cond.h"
#include "instructions/asm_fprintf.h"
#include "../parser_func/getters.h"
#include "../register_machine/reg_m.h"
#include "generators/stack_generator.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_REPEAT() {
    i_level_add(i_REPEAT);
    add_to_list(NULL, i_REPEAT);
}

void eval_REPEAT(i_graph **i_current) {
    if (i_level_is_empty_eval()) {
        i_graph_analyze_repeat_until(i_current);
    }

    oper_regs_store_drop();
    i_level_add_branch_eval(i_REPEAT, false, NULL);
    stack_ptr_clear();
}

void add_UNTIL(expression_t *expr) {
    eval_check_1(expr);
    eval_check_2(expr);

    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_POP);
        if (b_type == i_REPEAT) {
            add_to_list(expr, i_UNTIL);
            return;
        }
    }

    fprintf(stderr, "[REPEAT_UNTIL]: There is no matching REPEAT for UNTIL!\n");
    exit(EXIT_FAILURE);
}

static expr_type repeat_until_inverse_cond(expr_type type) {
    switch (type) {
        case cond_IS_EQUAL:
            return cond_IS_N_EQUAL;
            break;
        case cond_IS_N_EQUAL:
            return cond_IS_EQUAL;
            break;
        case cond_LESS:
            return cond_GREATER_EQ;
            break;
        case cond_GREATER:
            return cond_LESS_EQ;
            break;
        case cond_LESS_EQ:
            return cond_GREATER;
            break;
        case cond_GREATER_EQ:
            return cond_LESS;
            break;
        default:
            fprintf(stderr, "[COND_inv]: Wrong value of cond_type!\n");
            exit(EXIT_FAILURE);
    }
}

void eval_UNTIL(i_graph **i_current) {
    i_level i_repeat = i_level_pop_branch_eval(true);
    expression_t const * const expr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    if (!(assign_val_1.is_reg || assign_val_2.is_reg)) {
        bool cond = cond_val_from_const(assign_val_1.constant, assign_val_2.constant, expr->type);
        mpz_clear(assign_val_1.constant);
        mpz_clear(assign_val_2.constant);
        if (!cond) {
            fprintf(stderr, "[I_GRAPH]: Endless loop in code!\n");
            print_expression((expression_t *)expr, stderr);
            fprintf(stderr, "\n");
            exit(EXIT_FAILURE);
        }
    } else {
        const expr_type cond_type = repeat_until_inverse_cond(expr->type);
        reg *x = cond_val_from_vals(assign_val_1, assign_val_2, cond_type);
        oper_regs_store_drop();

        if (cond_type != cond_IS_EQUAL) {
            JZERO(x); // compare
        } else {
            JZERO_i_idx(x, 2);
            JUMP();
        }
        i_level_set_reserved_jump(i_repeat.reserved_jmp_idx,
                                  (int64_t)i_repeat.i_num + 1 - (int64_t)asm_get_i_num());
        stack_ptr_clear();
    }
}
