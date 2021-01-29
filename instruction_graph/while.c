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

        // SECOND UNWIND //
        assign_val_1 = oper_get_assign_val_1(expr);
        assign_val_2 = oper_get_assign_val_2(expr);
        if (assign_val_1.is_reg) {
            reg_m_promote(r_set, assign_val_1.reg->addr);
        }

        x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
        reg_m_drop_addr(r_set, COND_ADDR);
        x->addr = COND_ADDR;
        if (expr->type != cond_IS_EQUAL) {
            JZERO(x); // compare
        } else {
            JZERO_i_idx(x, 2);
            JUMP();
        }

        stack_ptr_clear();
        reg_m_drop_addr(r_set, VAL_GEN_ADDR);

        i_level_add_branch_eval(i_WHILE, true, (void *)expr); // SECOND JZERO
        *i_current = instruction_while->next;
        i_graph_execute(instruction_endwhile);
        *i_current = instruction_while;
        // SECOND UNWIND //

        // THIRD UNWIND PREP //
        assign_val_1 = oper_get_assign_val_1(expr);
        assign_val_2 = oper_get_assign_val_2(expr);
        if (assign_val_1.is_reg) {
            reg_m_promote(r_set, assign_val_1.reg->addr);
        }

        x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
        reg_m_drop_addr(r_set, COND_ADDR);
        x->addr = COND_ADDR;

        stack_ptr_clear();
        reg_m_drop_addr(r_set, VAL_GEN_ADDR);

        i_level i_while_2 = i_level_pop_branch_eval(false);
        reg_m_sort_by_snapshot(r_set, i_while_2.r_snap.r);

        bool total_store = false;
        for (int32_t i=0; i<REG_SIZE; ++i) {
            const bool regs_identic = r_set->r[i]->addr == i_while_2.r_snap.r[i].addr &&
                                      r_set->r[i]->id == i_while_2.r_snap.r[i].id &&
                                      r_set->r[i]->flags == i_while_2.r_snap.r[i].flags;
            const bool useless_addr = r_set->r[i]->addr > COND_ADDR && i_while_2.r_snap.r[i].addr > COND_ADDR;
            if (!(regs_identic || useless_addr)) {
                total_store = true;
                break;
            }
        }

        if (total_store) {
            oper_regs_store_drop();
            x->addr = COND_ADDR;
            stack_ptr_clear();
        }

        if (expr->type != cond_IS_EQUAL) {
            JZERO(x); // compare
        } else {
            JZERO_i_idx(x, 2);
            JUMP();
        }

        i_level_add_branch_eval(i_WHILE, !total_store, (void *)expr);
        // THIRD UNWIND PREP //
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
    i_level i_while_3 = i_level_pop_branch_eval(true);
    i_level i_while_2 = i_level_pop_branch_eval(true);
    i_level i_while_1 = i_level_pop_branch_eval(true);

    expression_t const * const expr = i_while_1.payload;
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
    reg_m_drop_addr(r_set, COND_ADDR);
    x->addr = COND_ADDR;

    if (!i_while_3.r_snap.have_mpz) { // total_store == true
        oper_regs_store_drop();
        reg *dest = NULL;
        for (int32_t i=0; i<REG_SIZE; ++i) {
            if (i_while_3.r_snap.r[i].addr == COND_ADDR) {
                dest = reg_m_get_by_id(r_set, i_while_3.r_snap.r[i].id);
            }
        }

        if (!dest) {
            fprintf(stderr, "[WHILE]: Endwhile got NULL-ptr on register search!\n");
            exit(EXIT_FAILURE);
        }

        oper_reg_swap(dest, x);
    }

    int64_t jump_loc = i_while_3.i_num - (asm_get_i_num() + 1);
    if (expr->type == cond_IS_EQUAL) {
        jump_loc -= 1;
    }
    JUMP_i_idx(jump_loc);

    // CLEARING FIRST JZERO
    i_level_set_reserved_jump(i_while_1.reserved_jmp_idx,
                              (asm_get_i_num() - i_while_1.i_num) + 1);
    reg_m_apply_snapshot(r_set, i_while_1.r_snap);
    oper_regs_store_drop();
    stack_ptr_clear();
    //////

    JUMP();
    i_level_add_branch_eval(i_ENDWHILE, false, NULL);
    i_level i_endwhile_1 = i_level_pop_branch_eval(true);

    // CLEARING SECOND JZERO
    i_level_set_reserved_jump(i_while_2.reserved_jmp_idx,
                              (asm_get_i_num() - i_while_2.i_num) + 1);
    reg_m_apply_snapshot(r_set, i_while_2.r_snap);
    oper_regs_store_drop();
    stack_ptr_clear();
    //////

    JUMP();
    i_level_add_branch_eval(i_ENDWHILE, false, NULL);
    i_level i_endwhile_2 = i_level_pop_branch_eval(true);

    // CLEARING THIRD JZERO
    i_level_set_reserved_jump(i_while_3.reserved_jmp_idx,
                              (asm_get_i_num() - i_while_3.i_num) + 1);
    reg_m_apply_snapshot(r_set, i_while_3.r_snap);
    oper_regs_store_drop();
    stack_ptr_clear();
    //////

    i_level_set_reserved_jump(i_endwhile_1.reserved_jmp_idx,
                              (asm_get_i_num() - i_endwhile_1.i_num) + 1);
    i_level_set_reserved_jump(i_endwhile_2.reserved_jmp_idx,
                              (asm_get_i_num() - i_endwhile_2.i_num) + 1);
}
