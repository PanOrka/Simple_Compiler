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

void add_IF(expression_t *expr) {
    eval_check_1(expr);
    eval_check_2(expr);

    i_level_add(i_IF);
    add_to_list(expr, i_IF);
}

void eval_IF(i_graph **i_current) {
    expression_t const * const expr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    if (!(assign_val_1.is_reg || assign_val_2.is_reg)) { // both constants
        bool cond = cond_val_from_const(assign_val_1.constant, assign_val_2.constant, expr->type);
        mpz_clear(assign_val_1.constant);
        mpz_clear(assign_val_2.constant);
        i_graph_clear_if(cond, i_current);
    } else {
        if (i_level_is_empty_eval()) {
            i_graph_analyze_if(i_current);
        }
        reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
        if (expr->type != cond_IS_EQUAL) {
            JZERO(x); // compare
        } else {
            JZERO_i_idx(x, 2);
            JUMP();
        }
        reg_m_drop_addr(r_set, TEMP_ADDR_1);
        reg_m_drop_addr(r_set, TEMP_ADDR_2);
        reg_m_drop_addr(r_set, TEMP_ADDR_3);
        i_level_add_branch_eval(i_IF, true, NULL);
    }
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

void eval_ELSE(i_graph **i_current) {
    oper_regs_store_drop();
    reg_set *r_set = get_reg_set();

    reg_snapshot r_snap = i_level_pop_branch_eval(false).r_snap;
    reg_m_apply_snapshot(r_set, r_snap);
    JUMP();
    i_level_add_branch_eval(i_ELSE, false, NULL);
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

void eval_ENDIF(i_graph **i_current) {
    if (i_level_pop_branch_eval(false).type == i_ELSE) {
        i_level i_else = i_level_pop_branch_eval(true);
        i_level i_if = i_level_pop_branch_eval(true);

        i_level_set_reserved_jump(i_if.reserved_jmp_idx,
                                  (i_else.i_num - i_if.i_num) + 1);

        // ENDING ELSE
        oper_regs_store_drop();
        reg_set *r_set = get_reg_set();
        i_level_set_reserved_jump(i_else.reserved_jmp_idx,
                                  (asm_get_i_num() - i_else.i_num) + 1);

        stack_ptr_clear();
    } else if (i_level_pop_branch_eval(false).type == i_IF) {
        i_level i_if = i_level_pop_branch_eval(true);
        oper_regs_store_drop();

        reg_set *r_set = get_reg_set();
        reg_m_apply_snapshot(r_set, i_if.r_snap);

        JUMP();
        i_level_add_branch_eval(i_ENDIF, false, NULL);
        i_level i_endif = i_level_pop_branch_eval(true);

        i_level_set_reserved_jump(i_if.reserved_jmp_idx,
                                  (i_endif.i_num - i_if.i_num) + 1);
        oper_regs_store_drop();
        i_level_set_reserved_jump(i_endif.reserved_jmp_idx,
                                  (asm_get_i_num() - i_endif.i_num) + 1);

        stack_ptr_clear();
    } else {
        fprintf(stderr, "[EVAL_ENDIF]: No matching type of branch!\n");
    }
}
