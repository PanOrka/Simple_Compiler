#include "../parser_func/expressions.h"
#include "i_level.h"
#include "expr_checker.h"
#include "std_oper/std_oper.h"
#include "conditions/cond.h"
#include "instructions/asm_fprintf.h"
#include "../parser_func/getters.h"
#include "../register_machine/reg_m.h"
#include "generators/stack_generator.h"
#include "generators/val_generator.h"

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

typedef struct {
    int32_t total_unwinds;
    bool same_registers;
} msg;

static reg * fetch_cond(expression_t const * const expr) {
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    reg *x = cond_val_from_vals(assign_val_1, assign_val_2, expr->type);
    reg_m_drop_addr(r_set, COND_ADDR);
    x->addr = COND_ADDR;

    return x;
}

static void add_branch(expression_t const * const expr, reg *x) {
    if (expr->type != cond_IS_EQUAL) {
        JZERO(x); // compare
    } else {
        JZERO_i_idx(x, 2);
        JUMP();
    }
    i_level_add_branch_eval(i_WHILE, true, (void *)expr);
}

static void while_unwind(i_graph **i_current,
                         i_graph *instruction_while,
                         i_graph *instruction_endwhile)
{
    *i_current = instruction_while->next;
    i_graph_execute(instruction_endwhile);
    *i_current = instruction_while;
}

static void set_last_jump_location(expression_t const * const expr, int64_t i_num) {
    int64_t jump_loc = i_num - (asm_get_i_num() + 1);
    if (expr->type == cond_IS_EQUAL) {
        jump_loc -= 1;
    }
    JUMP_i_idx(jump_loc);
}

static void end_while(const int32_t max_unwinds) {
    reg_set *r_set = get_reg_set();

    i_level i_while[MAX_WHILE_UNDWIND + 2];

    for (int32_t i=max_unwinds-1; i>=0; --i) {
        i_while[i] = i_level_pop_branch_eval(true);
    }

    int64_t delta[MAX_WHILE_UNDWIND + 1];

    for (int32_t i=0; i<max_unwinds-1; ++i) {
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
    i_level_set_reserved_jump(i_while[max_unwinds-1].reserved_jmp_idx,
                            (asm_get_i_num() - i_while[max_unwinds-1].i_num) + 1);
    reg_m_apply_snapshot(r_set, i_while[max_unwinds-1].r_snap);
    oper_regs_store_drop();
    stack_ptr_clear();

    for (int32_t i=max_unwinds-2; i>=0; --i) {
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

static void eval_known_regs(expression_t const * const expr,
                            i_graph **i_current,
                            i_graph *instruction_while,
                            i_graph *instruction_endwhile,
                            const int32_t max_unwinds)
{
    reg_set *r_set = get_reg_set();

    i_level last_while = i_level_pop_branch_eval(false);

    if (last_while.r_snap.stack_ptr_init) {
        stack_ptr_generate_from_mpz(last_while.r_snap.stack_ptr_value);
    }
    if (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated) {
        val_generate_from_mpz(last_while.r_snap.val_gen_value);
    }

    set_last_jump_location(expr, last_while.i_num);

    end_while(max_unwinds);
    *i_current = instruction_endwhile;
}

static void eval_with_regs_drop(expression_t const * const expr,
                                i_graph **i_current,
                                i_graph *instruction_while,
                                i_graph *instruction_endwhile,
                                const int32_t max_unwinds)
{
    reg_set *r_set = get_reg_set();

    reg *x = fetch_cond(expr);

    oper_regs_store_drop();
    stack_ptr_clear();
    x->addr = COND_ADDR;
    add_branch(expr, x);

    while_unwind(i_current, instruction_while, instruction_endwhile);
    x = fetch_cond(expr);
    oper_regs_store_drop();

    i_level last_while = i_level_pop_branch_eval(false);

    reg *dest = NULL;
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (last_while.r_snap.r[i].addr == COND_ADDR) {
            dest = reg_m_get_by_id(r_set, last_while.r_snap.r[i].id);
        }
    }

    if (!dest) {
        fprintf(stderr, "[WHILE]: Endwhile got NULL-ptr on register search!\n");
        exit(EXIT_FAILURE);
    }
    oper_reg_swap(dest, x);

    set_last_jump_location(expr, last_while.i_num);

    end_while(max_unwinds);
    *i_current = instruction_endwhile;
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
        add_branch(expr, x);
        
        while_unwind(i_current, instruction_while, instruction_endwhile);
        // FIRST UNWIND //

        // ADDITIONAL UNWINDS //
        bool same_registers = false;
        int32_t max_unwinds = MAX_WHILE_UNDWIND;

        i_level i_while_last;
        for (int32_t i=0; i<MAX_WHILE_UNDWIND; ++i) {
            i_while_last = i_level_pop_branch_eval(false);
            x = fetch_cond(expr);

            reg_m_sort_by_snapshot(r_set, i_while_last.r_snap.r);

            bool total_store = false;
            for (int32_t i=0; i<REG_SIZE; ++i) {
                const bool regs_identic = r_set->r[i]->addr == i_while_last.r_snap.r[i].addr &&
                                          r_set->r[i]->id == i_while_last.r_snap.r[i].id &&
                                          r_set->r[i]->flags == i_while_last.r_snap.r[i].flags;
                const bool useless_addr = r_set->r[i]->addr > COND_ADDR && r_set->r[i]->addr != VAL_GEN_ADDR &&
                                          i_while_last.r_snap.r[i].addr > COND_ADDR && i_while_last.r_snap.r[i].addr != VAL_GEN_ADDR;
                const bool same_val_gen = r_set->r[i]->addr == VAL_GEN_ADDR && i_while_last.r_snap.r[i].addr == VAL_GEN_ADDR;
                if (!(regs_identic || useless_addr || same_val_gen)) {
                    total_store = true;
                    break;
                }
            }

            if (!total_store) {
                if (i_while_last.r_snap.stack_ptr_init) {
                    stack_ptr_generate_from_mpz(i_while_last.r_snap.stack_ptr_value);
                }
                if (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated) {
                    val_generate_from_mpz(i_while_last.r_snap.val_gen_value);
                }
                reg_m_sort_by_snapshot(r_set, i_while_last.r_snap.r);
                same_registers = true;
                max_unwinds = i;
                break;
            } else {
                add_branch(expr, x);
                while_unwind(i_current, instruction_while, instruction_endwhile);
            }
        }
        // ADDITIONAL UNWINDS //

        // LAST UNWIND //
        if (same_registers) {
            eval_known_regs(expr,
                            i_current,
                            instruction_while,
                            instruction_endwhile,
                            max_unwinds + 1);
        } else {
            eval_with_regs_drop(expr,
                                i_current,
                                instruction_while,
                                instruction_endwhile,
                                max_unwinds + 2);
        }
        // LAST UNWIND //
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
    fprintf(stderr, "[WHILE]: While should be fully evaluated in eval_WHILE!\n");
    exit(EXIT_FAILURE);
}
