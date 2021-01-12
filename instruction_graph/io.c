#include "expr_checker.h"
#include "i_graph.h"
#include "../parser_func/getters.h"
#include "generators/stack_generator.h"
#include "generators/val_generator.h"
#include "std_oper/std_oper.h"
#include "instructions/asm_fprintf.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_READ(expression_t *expr) {
    eval_check_0(expr);
    add_to_list(expr, i_READ);
}

void eval_READ(i_graph **i_current) {
    expression_t const * const expr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    if ((expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) && !(expr->mask & ASSIGN_SYM2_NUM)) {
        const bool assign_sym_2_addr = expr->addr_mask & ASSIGN_SYM2_ADDR;
        const bool assign_sym_2_const = !assign_sym_2_addr && (expr->var_2[0].var->flags & SYMBOL_IS_CONSTANT);
        const bool assign_sym_1_const = expr->var_1[0].var->flags & SYMBOL_IS_CONSTANT;

        if (assign_sym_2_const && assign_sym_1_const) {
            array_value *arr_val = expr->var_1[0].var->consts.arr_value;

            mpz_t idx;
            mpz_init_set(idx, expr->var_2[0].var->consts.value);
            mpz_sub_ui(idx, idx, expr->var_1[0].var->_add_info.start_idx);

            const uint64_t idx_ui = mpz_get_ui(idx);
            mpz_clear(idx);
            arr_val = oper_arr_val_find(arr_val, idx_ui);

            addr_t const eff_addr = expr->var_1[0].var->addr[0] + idx_ui;

            reg_m_drop_addr(r_set, eff_addr);
            if (arr_val) {
                arr_val->is_in_memory = true;
                arr_val->is_constant = false;
                mpz_set_si(arr_val->value, 0);
            } else {
                array_value new_arr_val = {
                    .is_constant = false,
                    .is_in_memory = true,
                    .n = idx_ui
                };
                mpz_init(new_arr_val.value);
                oper_arr_val_add(expr->var_1[0].var, new_arr_val);
            }

            stack_ptr_generate(eff_addr);
        } else if (assign_sym_2_const) {
            mpz_t eff_addr;
            mpz_init_set_ui(eff_addr, expr->var_1[0].var->addr[0]);
            mpz_sub_ui(eff_addr, eff_addr, expr->var_1[0].var->_add_info.start_idx);
            mpz_add(eff_addr, eff_addr, expr->var_2[0].var->consts.value);

            const addr_t eff_addr_ui = mpz_get_ui(eff_addr);
            mpz_clear(eff_addr);

            reg_m_drop_addr(r_set, eff_addr_ui);
            stack_ptr_generate(eff_addr_ui);
        } else if (assign_sym_1_const && assign_sym_2_addr) { // during loops impl

        } else {
            if (assign_sym_1_const) {
                oper_flush_array_to_mem(expr->var_1[0].var);
                oper_arr_set_non_constant(expr->var_1[0].var);
            }
            oper_store_array(expr->var_1[0].var->addr);

            addr_t const var_idx_addr = (expr->addr_mask & ASSIGN_SYM2_ADDR) ? expr->var_2[0].addr : expr->var_2[0].var->addr[0];
            oper_set_stack_ptr_addr_arr(var_idx_addr,
                                        expr->var_1[0].var->addr[0],
                                        expr->var_1[0].var->_add_info.start_idx);
            oper_drop_array(expr->var_1[0].var->addr);
        }
    } else {
        const bool assign_sym_1_const = expr->var_1[0].var->flags & SYMBOL_IS_CONSTANT;
        const uint64_t idx = expr->var_2[0].num;
        addr_t const eff_addr = expr->var_1[0].var->addr[0] + idx;

        reg_m_drop_addr(r_set, eff_addr);

        if (assign_sym_1_const) {
            if (expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) {
                array_value *arr_val = expr->var_1[0].var->consts.arr_value;
                arr_val = oper_arr_val_find(arr_val, idx);

                if (arr_val) {
                    arr_val->is_in_memory = true;
                    arr_val->is_constant = false;
                    mpz_set_si(arr_val->value, 0);
                } else {
                    array_value new_arr_val = {
                        .is_constant = false,
                        .is_in_memory = true,
                        .n = idx
                    };
                    mpz_init(new_arr_val.value);
                    oper_arr_val_add(expr->var_1[0].var, new_arr_val);
                }
            } else {
                expr->var_1[0].var->flags &= ~SYMBOL_IS_CONSTANT;
                mpz_set_si(expr->var_1[0].var->consts.value, 0);
            }
        }

        stack_ptr_generate(eff_addr);
    }

    GET(&(r_set->stack_ptr));
}

void add_WRITE(expression_t *expr) {
    eval_check_1(expr);
    if (expr->mask & LEFT_SYM1_NUM) {
        symbol_table *s_table = get_symbol_table();
        symbol *sym = symbol_table_add(s_table, NULL, (add_info){ .start_idx = 0 }, 1, SYMBOL_NO_FLAGS);
        expr->var_1[0].addr = sym->addr[0];
        symbol_table_pop(s_table);
    }
    add_to_list(expr, i_WRITE);
}

static void write_store_set_from_const(symbol *var_1, symbol *var_2) {
    reg_set *r_set = get_reg_set();

    mpz_t eff_addr;
    mpz_init_set_ui(eff_addr, var_1->addr[0]);
    mpz_sub_ui(eff_addr, eff_addr, var_1->_add_info.start_idx);
    mpz_add(eff_addr, eff_addr, var_2->consts.value);

    const addr_t eff_addr_ui = mpz_get_ui(eff_addr);
    mpz_clear(eff_addr);

    stack_ptr_generate(eff_addr_ui);
    reg_allocator reg_alloc = reg_m_get(r_set, eff_addr_ui, false);
    if (reg_alloc.was_allocated && (reg_alloc.r->flags & REG_MODIFIED)) {
        STORE(reg_alloc.r, &(r_set->stack_ptr));
        reg_alloc.r->flags &= ~REG_MODIFIED;
    }
}

void eval_WRITE(i_graph **i_current) {
    expression_t const * const expr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    const bool left_sym_1_addr = expr->addr_mask & LEFT_SYM1_ADDR;
    if (left_sym_1_addr) {
        stack_ptr_generate(expr->var_1[1].addr);
        reg_allocator reg_alloc = reg_m_get(r_set, expr->var_1[1].addr, false);
        if (reg_alloc.was_allocated && (reg_alloc.r->flags & REG_MODIFIED)) {
            STORE(reg_alloc.r, &(r_set->stack_ptr));
            reg_alloc.r->flags &= ~REG_MODIFIED;
        }
    } else if (!(expr->mask & LEFT_SYM1_NUM)) {
        if ((expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) && !(expr->mask & LEFT_SYM2_NUM)) {
            const bool left_sym_2_addr = expr->addr_mask & LEFT_SYM2_ADDR;
            const bool left_sym_2_const = !left_sym_2_addr && (expr->var_2[1].var->flags & SYMBOL_IS_CONSTANT);
            const bool left_sym_1_const = expr->var_1[1].var->flags & SYMBOL_IS_CONSTANT;

            if (left_sym_2_const && left_sym_1_const) {
                array_value *arr_val = expr->var_1[1].var->consts.arr_value;

                mpz_t idx;
                mpz_init_set(idx, expr->var_2[1].var->consts.value);
                mpz_sub_ui(idx, idx, expr->var_1[1].var->_add_info.start_idx);

                const uint64_t idx_ui = mpz_get_ui(idx);
                mpz_clear(idx);
                arr_val = oper_arr_val_find(arr_val, idx_ui);

                if (arr_val) {
                    if (arr_val->is_constant) {
                        reg *val_reg = val_generate_from_mpz(arr_val->value);
                        stack_ptr_generate(expr->var_1[1].var->addr[0] + idx_ui);
                        STORE(val_reg, &(r_set->stack_ptr));
                        arr_val->is_in_memory = true;
                    } else {
                        write_store_set_from_const(expr->var_1[1].var, expr->var_2[1].var);
                    }
                } else {
                    stack_ptr_generate(expr->var_1[1].var->addr[0] + idx_ui);
                }
            } else if (left_sym_2_const) {
                write_store_set_from_const(expr->var_1[1].var, expr->var_2[1].var);
            } else if (left_sym_1_const && left_sym_2_addr) { // during loops impl
            
            } else {
                if (left_sym_1_const) {
                    oper_flush_array_to_mem(expr->var_1[1].var);
                }
                oper_store_array(expr->var_1[1].var->addr);

                addr_t const var_idx_addr = (expr->addr_mask & LEFT_SYM2_ADDR) ? expr->var_2[1].addr : expr->var_2[1].var->addr[0];
                oper_set_stack_ptr_addr_arr(var_idx_addr,
                                            expr->var_1[1].var->addr[0],
                                            expr->var_1[1].var->_add_info.start_idx);
            }
        } else {
            const bool left_sym_1_const = expr->var_1[1].var->flags & SYMBOL_IS_CONSTANT;

            if (left_sym_1_const) {
                if (expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
                    array_value *arr_val = expr->var_1[1].var->consts.arr_value;
                    const uint64_t idx = expr->var_2[1].num;
                    arr_val = oper_arr_val_find(arr_val, idx);

                    if (arr_val) {
                        if (arr_val->is_constant) {
                            if (arr_val->is_in_memory) {
                                stack_ptr_generate(expr->var_1[1].var->addr[0] + idx);
                            } else {
                                reg *val_reg = val_generate_from_mpz(arr_val->value);
                                stack_ptr_generate(expr->var_1[1].var->addr[0] + idx);
                                STORE(val_reg, &(r_set->stack_ptr));
                                arr_val->is_in_memory = true;
                            }
                        } else {
                            addr_t const eff_addr = expr->var_1[1].var->addr[0] + idx;
                            stack_ptr_generate(eff_addr);
                            reg_allocator reg_alloc = reg_m_get(r_set, eff_addr, false);
                            if (reg_alloc.was_allocated && (reg_alloc.r->flags & REG_MODIFIED)) {
                                STORE(reg_alloc.r, &(r_set->stack_ptr));
                                reg_alloc.r->flags &= ~REG_MODIFIED;
                            }
                        }
                    } else {
                        stack_ptr_generate(expr->var_1[1].var->addr[0] + idx);
                    }
                } else {
                    if (expr->var_1[1].var->symbol_in_memory) {
                        stack_ptr_generate(expr->var_1[1].var->addr[0]);
                    } else {
                        reg *val_reg = val_generate_from_mpz(expr->var_1[1].var->consts.value);
                        stack_ptr_generate(expr->var_1[1].var->addr[0]);
                        STORE(val_reg, &(r_set->stack_ptr));
                        expr->var_1[1].var->symbol_in_memory = true;
                    }
                }
            } else {
                addr_t const eff_addr = expr->var_1[1].var->addr[0] + (addr_t)expr->var_2[1].num;
                stack_ptr_generate(eff_addr);
                reg_allocator reg_alloc = reg_m_get(r_set, eff_addr, false);
                if (reg_alloc.was_allocated && (reg_alloc.r->flags & REG_MODIFIED)) {
                    STORE(reg_alloc.r, &(r_set->stack_ptr));
                    reg_alloc.r->flags &= ~REG_MODIFIED;
                }
            }
        }
    } else {
        addr_t const temp_write_addr = expr->var_1[0].addr;
        reg *write_reg = val_generate(expr->var_1[1].num);
        stack_ptr_generate(temp_write_addr);
        STORE(write_reg, &(r_set->stack_ptr));
    }

    PUT(&(r_set->stack_ptr));
}
