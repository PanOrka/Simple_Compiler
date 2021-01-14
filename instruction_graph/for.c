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

    oper_regs_store_drop();

    JZERO(range); // compare
    stack_ptr_clear();
    range->addr = TEMP_ADDR_1;
    i_level_add_branch_eval(i_FOR, false, (void *)loop_info);
    reg_m_drop_addr(r_set, TEMP_ADDR_1);
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
    i_level i_for = i_level_pop_branch_eval(true);
    for_loop_t const * const loop_info = i_for.payload;
    reg_set *r_set = get_reg_set();

    reg_allocator iter_alloc = oper_get_reg_for_variable(loop_info->iterator);
    if (!iter_alloc.was_allocated) {
        oper_load_variable_to_reg(iter_alloc.r, loop_info->iterator);
    }
    reg *iter = iter_alloc.r;

    reg_allocator range_alloc = oper_get_reg_for_variable(loop_info->range);
    if (!range_alloc.was_allocated) {
        oper_load_variable_to_reg(range_alloc.r, loop_info->range);
    }
    reg *range = range_alloc.r;

    if (loop_info->type == loop_TO) {
        INC(iter);
    } else {
        DEC(iter);
    }
    DEC(range);
    iter->flags = REG_MODIFIED;
    range->flags = REG_MODIFIED;

    oper_regs_store_drop();

    const reg_snapshot r_snap = i_for.r_snap;

    reg *dest_range = NULL;
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_snap.r[i].addr == TEMP_ADDR_1) {
            dest_range = reg_m_get_by_id(r_set, r_snap.r[i].id);
        }
    }

    if (!dest_range) {
        fprintf(stderr, "[WHILE]: Endfor got NULL-ptr on register search!\n");
        exit(EXIT_FAILURE);
    }

    oper_reg_swap(dest_range, range);

    const int64_t jump_loc = i_for.i_num - (asm_get_i_num() + 1);
    JUMP_i_idx(jump_loc);

    i_level_set_reserved_jump(i_for.reserved_jmp_idx,
                              (asm_get_i_num() - i_for.i_num) + 1);
    stack_ptr_clear();
}
