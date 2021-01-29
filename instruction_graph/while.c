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

void i_graph_set_start(i_graph *new_start);

#ifndef MAX_WHILE_UNWIND
#define MAX_WHILE_UNDWIND 100
#endif

static void while_unwind_additional(expression_t const * const expr,
                                    i_graph **i_current,
                                    i_graph *instruction_while,
                                    i_graph *instruction_endwhile)
{
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
    reg_m_drop_addr(r_set, COND_ADDR);
    x->addr = COND_ADDR;
    if (expr->type != cond_IS_EQUAL) {
        JZERO(x); // compare
    } else {
        JZERO_i_idx(x, 2);
        JUMP();
    }
    i_level_add_branch_eval(i_WHILE, true, (void *)expr);
    *i_current = instruction_while->next;
    i_graph_execute(instruction_endwhile);
    *i_current = instruction_while;
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
        i_graph *instruction_while = *i_current;
        i_graph *instruction_endwhile = NULL;
        i_graph_while_find(instruction_while, &instruction_endwhile);
        if (!instruction_endwhile) {
            fprintf(stderr, "[EVAL_WHILE]: While-find NULLptr!\n");
            exit(EXIT_FAILURE);
        }

        // FIRST UNWIND //
        reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
        reg_m_drop_addr(r_set, COND_ADDR);
        x->addr = COND_ADDR;
        if (expr->type != cond_IS_EQUAL) {
            JZERO(x); // compare
        } else {
            JZERO_i_idx(x, 2);
            JUMP();
        }

        i_level_add_branch_eval(i_WHILE, true, (void *)expr); // FIRST JZERO
        *i_current = instruction_while->next;
        i_graph_execute(instruction_endwhile);
        *i_current = instruction_while;
        // FIRST UNWIND //

        // ADDITIONAL UNWINDS //
        for (int32_t i=0; i<MAX_WHILE_UNDWIND; ++i) {
            while_unwind_additional(expr, i_current, instruction_while, instruction_endwhile);
        }
        // ADDITIONAL UNWINDS //

        // LAST UNWIND PREP //
        assign_val_1 = oper_get_assign_val_1(expr);
        assign_val_2 = oper_get_assign_val_2(expr);
        if (assign_val_1.is_reg) {
            reg_m_promote(r_set, assign_val_1.reg->addr);
        }

        x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
        stack_ptr_clear();
        oper_regs_store_drop();
        x->addr = COND_ADDR;

        if (expr->type != cond_IS_EQUAL) {
            JZERO(x); // compare
        } else {
            JZERO_i_idx(x, 2);
            JUMP();
        }

        i_level_add_branch_eval(i_WHILE, false, (void *)expr);
        // LAST UNWIND PREP //
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
    const int32_t n_all_undwinds = MAX_WHILE_UNDWIND+2;
    i_level i_while[n_all_undwinds];

    for (int32_t i=n_all_undwinds-1; i>=0; --i) {
        i_while[i] = i_level_pop_branch_eval(true);
    }

    expression_t const * const expr = i_while[0].payload;
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
    reg_m_drop_addr(r_set, COND_ADDR);
    x->addr = COND_ADDR;

    oper_regs_store_drop();
    reg *dest = NULL;
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (i_while[n_all_undwinds-1].r_snap.r[i].addr == COND_ADDR) {
            dest = reg_m_get_by_id(r_set, i_while[n_all_undwinds-1].r_snap.r[i].id);
        }
    }

    if (!dest) {
        fprintf(stderr, "[WHILE]: Endwhile got NULL-ptr on register search!\n");
        exit(EXIT_FAILURE);
    }

    oper_reg_swap(dest, x);

    int64_t jump_loc = i_while[n_all_undwinds-1].i_num - (asm_get_i_num() + 1);
    if (expr->type == cond_IS_EQUAL) {
        jump_loc -= 1;
    }
    JUMP_i_idx(jump_loc);

    int64_t delta[n_all_undwinds-1];
    for (int32_t i=0; i<n_all_undwinds-1; ++i) {
        delta[i] = asm_get_i_num();
        i_level_set_reserved_jump(i_while[i].reserved_jmp_idx,
                                  (asm_get_i_num() - i_while[i].i_num) + 1);
        reg_m_apply_snapshot(r_set, i_while[i].r_snap);
        oper_regs_store_drop();
        stack_ptr_clear();

        delta[i] = asm_get_i_num() - delta[i];
        if (delta[i] > 0) {
            JUMP();
            i_level_add_branch_eval(i_ENDWHILE, false, NULL);
        }
    }
    i_level_set_reserved_jump(i_while[n_all_undwinds-1].reserved_jmp_idx,
                              (asm_get_i_num() - i_while[n_all_undwinds-1].i_num) + 1);
    reg_m_apply_snapshot(r_set, i_while[n_all_undwinds-1].r_snap);
    oper_regs_store_drop();
    stack_ptr_clear();

    for (int32_t i=n_all_undwinds-2; i>=0; --i) {
        if (delta[i] > 0) {
            i_level i_endwhile = i_level_pop_branch_eval(true);
            i_level_set_reserved_jump(i_endwhile.reserved_jmp_idx,
                                      (asm_get_i_num() - i_endwhile.i_num) + 1);
        } else {
            i_level_set_reserved_jump(i_while[i].reserved_jmp_idx,
                                      (asm_get_i_num() - i_while[i].i_num) + 1);
        }
    }
}
