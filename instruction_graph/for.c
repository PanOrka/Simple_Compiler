#include "../parser_func/loops.h"
#include "i_level.h"
#include "expr_checker.h"
#include "../parser_func/getters.h"
#include "conditions/cond.h"
#include "instructions/asm_fprintf.h"
#include "../register_machine/reg_m.h"
#include "generators/stack_generator.h"
#include "generators/val_generator.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_FOR(for_loop_t *loop) {
    eval_check_1(&(loop->range_vars));
    eval_check_2(&(loop->range_vars));

    i_level_add(i_FOR);
    add_to_list(loop, i_FOR);
}

static void num_sub(mpz_t dest, mpz_t src_1, mpz_t src_2) {
    mpz_sub(dest, src_1, src_2);
    if (mpz_cmp_si(dest, 0) < 0) {
        mpz_set_si(dest, 0);
    }
}

#ifndef MAX_FOR_UNWIND
#define MAX_FOR_UNWIND 100
#endif

static reg * fetch_cond(for_loop_t const * const loop_info) {
    reg_set *r_set = get_reg_set();

    reg_allocator range_alloc = oper_get_reg_for_variable(loop_info->range);
    if (!range_alloc.was_allocated) {
        oper_load_variable_to_reg(range_alloc.r, loop_info->range);
    }

    return range_alloc.r;
}

static reg * fetch_iter(for_loop_t const * const loop_info) {
    reg_set *r_set = get_reg_set();

    reg_allocator range_alloc = oper_get_reg_for_variable(loop_info->iterator);
    if (!range_alloc.was_allocated) {
        oper_load_variable_to_reg(range_alloc.r, loop_info->iterator);
    }

    return range_alloc.r;
}

static void change_iter(for_loop_t const * const loop_info,
                        reg *iter, reg *range)
{
    if (loop_info->type == loop_TO) {
        INC(iter);
    } else {
        DEC(iter);
    }
    DEC(range);
    iter->flags = REG_MODIFIED;
    range->flags = REG_MODIFIED;
}

static void for_unwind(i_graph **i_current,
                       i_graph *instruction_for,
                       i_graph *instruction_endfor)
{
    *i_current = instruction_for->next;
    i_graph_execute(instruction_endfor);
    *i_current = instruction_endfor;
}

static void add_branch(for_loop_t const * const loop_info, reg *x) {
    JZERO(x);
    i_level_add_branch_eval(i_FOR, true, (void *)loop_info);
}

static void set_last_jump_location(int64_t i_num) {
    int64_t jump_loc = i_num - (asm_get_i_num() + 1);
    JUMP_i_idx(jump_loc);
}

static void end_for(const int32_t max_unwinds) {
    reg_set *r_set = get_reg_set();

    i_level i_for[MAX_FOR_UNWIND + 2];

    for (int32_t i=max_unwinds-1; i>=0; --i) {
        i_for[i] = i_level_pop_branch_eval(true);
    }

    int64_t delta[MAX_FOR_UNWIND + 1];

    for (int32_t i=0; i<max_unwinds-1; ++i) {
        delta[i] = asm_get_i_num();
        i_level_set_reserved_jump(i_for[i].reserved_jmp_idx,
                                  (asm_get_i_num() - i_for[i].i_num) + 1);
        reg_m_apply_snapshot(r_set, i_for[i].r_snap);
        oper_regs_store_drop();
        stack_ptr_clear();

        delta[i] = asm_get_i_num() - delta[i];
        if (delta[i] > 0) {
            JUMP();
            i_level_add_branch_eval(i_ENDFOR, false, NULL);
        }
    }
    i_level_set_reserved_jump(i_for[max_unwinds-1].reserved_jmp_idx,
                              (asm_get_i_num() - i_for[max_unwinds-1].i_num) + 1);
    reg_m_apply_snapshot(r_set, i_for[max_unwinds-1].r_snap);
    oper_regs_store_drop();
    stack_ptr_clear();

    for (int32_t i=max_unwinds-2; i>=0; --i) {
        if (delta[i] > 0) {
            i_level i_endfor = i_level_pop_branch_eval(true);
            i_level_set_reserved_jump(i_endfor.reserved_jmp_idx,
                                      (asm_get_i_num() - i_endfor.i_num) + 1);
        } else {
            i_level_set_reserved_jump(i_for[i].reserved_jmp_idx,
                                      (asm_get_i_num() - i_for[i].i_num) + 1);
        }
    }
}

static void eval_known_regs(i_graph **i_current,
                            i_graph *instruction_for,
                            i_graph *instruction_endfor,
                            const int32_t max_unwinds)
{
    reg_set *r_set = get_reg_set();

    i_level last_for = i_level_pop_branch_eval(false);

    if (last_for.r_snap.stack_ptr_init) {
        stack_ptr_generate_from_mpz(last_for.r_snap.stack_ptr_value);
    }
    if (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated) {
        val_generate_from_mpz(last_for.r_snap.val_gen_value);
    }

    set_last_jump_location(last_for.i_num);

    end_for(max_unwinds);
    *i_current = instruction_endfor;
}

static void eval_with_regs_drop(for_loop_t const * const loop_info,
                                i_graph **i_current,
                                i_graph *instruction_for,
                                i_graph *instruction_endfor,
                                const int32_t max_unwinds)
{
    reg_set *r_set = get_reg_set();

    reg *iter = fetch_iter(loop_info);
    reg *range = fetch_cond(loop_info);
    change_iter(loop_info, iter, range);

    oper_regs_store_drop();
    stack_ptr_clear();

    range->addr = COND_ADDR;
    add_branch(loop_info, range);
    for_unwind(i_current, instruction_for, instruction_endfor);

    iter = fetch_iter(loop_info);
    range = fetch_cond(loop_info);
    change_iter(loop_info, iter, range);
    oper_regs_store_drop();

    i_level last_for = i_level_pop_branch_eval(false);

    reg *dest = NULL;
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (last_for.r_snap.r[i].addr == COND_ADDR) {
            dest = reg_m_get_by_id(r_set, last_for.r_snap.r[i].id);
        }
    }

    if (!dest) {
        fprintf(stderr, "[WHILE]: Endwhile got NULL-ptr on register search!\n");
        exit(EXIT_FAILURE);
    }
    oper_reg_swap(dest, range);

    set_last_jump_location(last_for.i_num);

    end_for(max_unwinds);
    *i_current = instruction_endfor;
}

void eval_FOR(i_graph **i_current) {
    for_loop_t const * const loop_info = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    val assign_val_1 = oper_get_assign_val_1(&(loop_info->range_vars));
    val assign_val_2 = oper_get_assign_val_2(&(loop_info->range_vars));
    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    if (i_level_is_empty_eval()) {
        i_graph_analyze_for(i_current);
    }

    reg *iter = NULL;
    reg *range = NULL;
    if (assign_val_1.is_reg || assign_val_2.is_reg) {
        if (!assign_val_1.is_reg) {
            assign_val_1.reg = val_generate_from_mpz(assign_val_1.constant);
            mpz_clear(assign_val_1.constant);
        } else {
            if (assign_val_1.reg->flags & REG_MODIFIED) {
                stack_ptr_generate(assign_val_1.reg->addr);
                STORE(assign_val_1.reg, &(r_set->stack_ptr));
                assign_val_1.reg->flags &= ~REG_MODIFIED;
            }
        }

        if (!assign_val_2.is_reg) {
            assign_val_2.reg = val_generate_from_mpz(assign_val_2.constant);
            mpz_clear(assign_val_2.constant);
        } else {
            if (assign_val_2.reg->flags & REG_MODIFIED) {
                stack_ptr_generate(assign_val_2.reg->addr);
                STORE(assign_val_2.reg, &(r_set->stack_ptr));
                assign_val_2.reg->flags &= ~REG_MODIFIED;
            }
        }

        if (assign_val_1.reg != assign_val_2.reg) {
            if (loop_info->type == loop_TO) {
                iter = assign_val_1.reg;
                range = assign_val_2.reg;
                INC(range);
                SUB(range, iter);
            } else {
                iter = oper_get_reg_for_variable(loop_info->iterator).r;
                oper_reg_swap(iter, assign_val_1.reg);
                INC(assign_val_1.reg);
                SUB(assign_val_1.reg, assign_val_2.reg);
                range = assign_val_1.reg;
            }
        } else {
            iter = assign_val_1.reg;
            range = oper_get_reg_for_variable(loop_info->range).r;
            RESET(range);
            INC(range);
        }
    } else {
        mpz_t ran;
        mpz_init(ran);

        if (loop_info->type == loop_TO) {
            iter = val_generate_from_mpz(assign_val_1.constant);
            iter->addr = loop_info->iterator;
            mpz_add_ui(assign_val_2.constant, assign_val_2.constant, 1);
            num_sub(ran, assign_val_2.constant, assign_val_1.constant);
            range = val_generate_from_mpz(ran);
        } else {
            iter = val_generate_from_mpz(assign_val_1.constant);
            iter->addr = loop_info->iterator;
            mpz_add_ui(assign_val_1.constant, assign_val_1.constant, 1);
            num_sub(ran, assign_val_1.constant, assign_val_2.constant);
            range = val_generate_from_mpz(ran);
        }
        mpz_clear(ran);
        mpz_clear(assign_val_1.constant);
        mpz_clear(assign_val_2.constant);
    }

    iter->addr = loop_info->iterator;
    iter->flags = REG_MODIFIED;
    range->addr = loop_info->range;
    range->flags = REG_MODIFIED;

    i_graph *instruction_for = *i_current;
    i_graph *instruction_endfor = NULL;
    i_graph_for_find(instruction_for, &instruction_endfor);
    if (!instruction_endfor) {
        fprintf(stderr, "[EVAL_FOR]: FOR-find NULLptr!\n");
        exit(EXIT_FAILURE);
    }

    // FIRST UNWIND //
    add_branch(loop_info, range);
    for_unwind(i_current, instruction_for, instruction_endfor);
    // FIRST UNWIND //

    // ADDITIONAL UNWINDS //
    bool same_registers = false;
    int32_t max_unwinds = MAX_FOR_UNWIND;

    for (int32_t i=0; i<MAX_FOR_UNWIND; ++i) {
        i_level i_for_last = i_level_pop_branch_eval(false);
        iter = fetch_iter(loop_info);
        range = fetch_cond(loop_info);
        change_iter(loop_info, iter, range);

        reg_m_sort_by_snapshot(r_set, i_for_last.r_snap.r);

        bool total_store = false;
        for (int32_t i=0; i<REG_SIZE; ++i) {
            const bool regs_identic = r_set->r[i]->addr == i_for_last.r_snap.r[i].addr &&
                                      r_set->r[i]->id == i_for_last.r_snap.r[i].id &&
                                      r_set->r[i]->flags == i_for_last.r_snap.r[i].flags;
            const bool useless_addr = r_set->r[i]->addr > COND_ADDR && r_set->r[i]->addr != VAL_GEN_ADDR &&
                                      i_for_last.r_snap.r[i].addr > COND_ADDR && i_for_last.r_snap.r[i].addr != VAL_GEN_ADDR;
            const bool same_val_gen = r_set->r[i]->addr == VAL_GEN_ADDR && i_for_last.r_snap.r[i].addr == VAL_GEN_ADDR;
            if (!(regs_identic || useless_addr || same_val_gen)) {
                total_store = true;
                break;
            }
        }

        if (!total_store) {
            same_registers = true;
            max_unwinds = i;
            break;
        } else {
            add_branch(loop_info, range);
            for_unwind(i_current, instruction_for, instruction_endfor);
        }
    }

    // LAST UNWIND //
    if (same_registers) {
        eval_known_regs(i_current,
                        instruction_for,
                        instruction_endfor,
                        max_unwinds + 1);
    } else {
        eval_with_regs_drop(loop_info,
                            i_current,
                            instruction_for,
                            instruction_endfor,
                            max_unwinds + 2);
    }
}

void add_ENDFOR() {
    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_POP);
        if (b_type == i_FOR) {
            add_to_list(NULL, i_ENDFOR);

            symbol_table *s_table = get_symbol_table();
            int32_t ctr = 0;
            while (true) {
                symbol popped = symbol_table_pop(s_table);
                free((void *)popped.identifier);
                ++ctr;

                if (popped.flags & SYMBOL_IS_ITER) {
                    if (popped.flags & SYMBOL_HAS_HIDE) {
                        symbol *hide = symbol_table_find_by_idx(s_table, popped._add_info.hide_idx);
                        hide->flags |= SYMBOL_NO_HIDDEN;
                    }

                    if (ctr != 2) {
                        fprintf(stderr, "[FOR]: Incorrect value of popped values counter: %d!\n", ctr);
                        exit(EXIT_FAILURE);
                    }

                    break;
                }

                if (ctr > 2) {
                    fprintf(stderr, "[FOR]: Counter of popped values > 2!\n");
                    exit(EXIT_FAILURE);
                }
            }
            return;
        }
    }

    fprintf(stderr, "[FOR]: There is no matching FOR for ENDFOR!\n");
    exit(EXIT_FAILURE);
}

void eval_ENDFOR(i_graph **i_current) {
    fprintf(stderr, "[FOR]: FOR should be fully evaluated in eval_FOR!\n");
    exit(EXIT_FAILURE);
}
