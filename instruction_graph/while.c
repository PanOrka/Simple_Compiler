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

void add_WHILE(expression_t *expr) {
    eval_check_1(expr);
    eval_check_2(expr);

    i_level_add(i_WHILE);
    add_to_list(expr, i_WHILE);
}

void eval_WHILE(i_graph **i_current) {
    expression_t const * const expr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    if (i_level_is_empty_eval()) {
        i_graph_analyze_while(i_current);
    }

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    if (!(assign_val_1.is_reg || assign_val_2.is_reg)) { // both constants
        bool cond = cond_val_from_const(assign_val_1.constant, assign_val_2.constant, expr->type);

        mpz_clear(assign_val_1.constant);
        mpz_clear(assign_val_2.constant);
        i_graph_clear_while(cond, i_current);
    } else {
        reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
        oper_regs_store_drop();
        if (expr->type != cond_IS_EQUAL) {
            JZERO(x); // compare
        } else {
            JZERO_i_idx(x, 2);
            JUMP();
        }

        x->addr = TEMP_ADDR_1;
        i_level_add_branch_eval(i_WHILE, false, (void *)expr);
        reg_m_drop_addr(r_set, TEMP_ADDR_1);
        stack_ptr_clear();
    }
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

void eval_ENDWHILE(i_graph **i_current) {
    i_level i_while = i_level_pop_branch_eval(true);
    expression_t const * const expr = i_while.payload;
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
    oper_regs_store_drop();

    const reg_snapshot r_snap = i_while.r_snap;

    reg *dest = NULL;
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_snap.r[i].addr == TEMP_ADDR_1) {
            dest = reg_m_get_by_id(r_set, r_snap.r[i].id);
        }
    }

    if (!dest) {
        fprintf(stderr, "[WHILE]: Endwhile got NULL-ptr on register search!\n");
        exit(EXIT_FAILURE);
    }

    oper_reg_swap(dest, x);

    int64_t jump_loc = i_while.i_num - (asm_get_i_num() + 1);
    if (expr->type == cond_IS_EQUAL) {
        jump_loc -= 1;
    }
    JUMP_i_idx(jump_loc);

    i_level_set_reserved_jump(i_while.reserved_jmp_idx,
                              (asm_get_i_num() - i_while.i_num) + 1);
    stack_ptr_clear();
}
