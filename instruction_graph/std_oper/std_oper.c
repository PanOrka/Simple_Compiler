#include "std_oper.h"
#include "../generators/stack_generator.h"
#include "../generators/val_generator.h"
#include "../../parser_func/getters.h"
#include "../instructions/asm_fprintf.h"

reg_allocator oper_get_reg_for_variable(addr_t addr) {
    reg_set *r_set = get_reg_set();
    reg_allocator r_alloc;
    if (addr == ADDR_UNDEF) {
        r_alloc = reg_m_LRU(r_set, false);
    } else {
        r_alloc = reg_m_get(r_set, addr, true);
    }

    if (!r_alloc.was_allocated) {
        if (r_alloc.r->flags & REG_MODIFIED) {
            stack_ptr_generate(r_alloc.r->addr);
            STORE(r_alloc.r, &(r_set->stack_ptr));
        }

        r_alloc.r->addr = ADDR_UNDEF;
        r_alloc.r->flags = REG_NO_FLAGS;
    }

    return r_alloc;
}

void oper_load_variable_to_reg(reg *r, addr_t addr) {
    reg_set *r_set = get_reg_set();
    stack_ptr_generate(addr);
    LOAD(r, &(r_set->stack_ptr));
    r->addr = addr;
}

static void add_sub_val_gen(reg *stack_ptr, addr_t arr_addr, reg *var_idx, uint64_t start_idx) {
    stack_ptr_generate(arr_addr);
    ADD(stack_ptr, var_idx);
    stack_ptr_clear();
    reg *s_idx = val_generate(start_idx);
    SUB(stack_ptr, s_idx);
}

static void dec_val_gen(reg *stack_ptr, addr_t arr_addr, reg *var_idx, num_t eff_addr) {
    stack_ptr_generate(arr_addr);
    ADD(stack_ptr, var_idx);
    stack_ptr_clear();
    for (int i=eff_addr; i < 0; ++i) {
        DEC(stack_ptr);
    }
}

void oper_set_stack_ptr_addr_arr(addr_t var_idx_addr, addr_t arr_addr, uint64_t start_idx) {
    reg_set *r_set = get_reg_set();

    reg_allocator var_idx = oper_get_reg_for_variable(var_idx_addr);
    if (!var_idx.was_allocated) {
        oper_load_variable_to_reg(var_idx.r, var_idx_addr);
    }

    num_t const eff_addr = (num_t)arr_addr - (num_t)start_idx;
    if (eff_addr >= 0) {
        stack_ptr_generate(eff_addr);
        ADD(&(r_set->stack_ptr), var_idx.r);
        stack_ptr_clear();
    } else if (eff_addr >= -5) {
        dec_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, eff_addr);
    } else if (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated) {
        add_sub_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, start_idx);
    } else if (!(reg_m_get(r_set, VAL_GEN_ADDR, false).r->flags & REG_MODIFIED)) {
        add_sub_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, start_idx);
    } else if (eff_addr >= -20) {
        dec_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, eff_addr);
    } else {
        add_sub_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, start_idx);
    }
}

void oper_reg_swap(reg *r1, reg *r2) {
    RESET(r1);
    ADD(r1, r2);
}

void oper_store_array(const addr_t addr[2]) {
    reg_set *r_set = get_reg_set();

    for (int32_t i=0; i<REG_SIZE; ++i) {
        reg * const r = r_set->r[i];
        if (r->addr >= addr[0] && r->addr < addr[1]) {
            if (r->flags & REG_MODIFIED) {
                stack_ptr_generate(r->addr);
                STORE(r, &(r_set->stack_ptr));
                r->flags &= ~REG_MODIFIED;
            }
        }
    }
}

void oper_drop_array(const addr_t addr[2]) {
    reg_set *r_set = get_reg_set();

    for (int32_t i=0; i<REG_SIZE; ++i) {
        reg * const r = r_set->r[i];
        if (r->addr >= addr[0] && r->addr < addr[1]) {
            reg_m_drop_addr(r_set, r->addr);
        }
    }
}

static void oper_flush_array_to_mem(symbol *arr) {
    reg_set *r_set = get_reg_set();
    array_value *arr_val = arr->consts.arr_value;
    while (arr_val) {
        if (!(arr_val->is_in_memory)) {
            reg *val_reg = val_generate_from_mpz(arr_val->value);
            stack_ptr_generate(arr->addr[0] + arr_val->n);
            STORE(val_reg, &(r_set->stack_ptr));
            arr_val->is_in_memory = true;
        }
        arr_val = arr_val->next;
    }
}

void oper_set_assign_val_0(expression_t const * const expr,
                           reg *assign_val,
                           uint8_t assign_val_flags)
{
    reg_set *r_set = get_reg_set();
    if ((expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) && !(expr->mask & ASSIGN_SYM2_NUM)) {
        oper_store_array(expr->var_1[0].var->addr);

        addr_t const var_idx_addr = (expr->addr_mask & ASSIGN_SYM2_ADDR) ? expr->var_2[0].addr : expr->var_2[0].var->addr[0];
        oper_set_stack_ptr_addr_arr(var_idx_addr,
                                    expr->var_1[0].var->addr[0],
                                    expr->var_1[0].var->_add_info.start_idx);

        oper_drop_array(expr->var_1[0].var->addr);

        STORE(assign_val, &(r_set->stack_ptr));
        if (assign_val_flags & ASSIGN_VAL_STASH) {
            reg_m_drop_addr(r_set, assign_val->addr);
        }
    } else {
        addr_t const eff_addr = expr->var_1[0].var->addr[0] + (addr_t)expr->var_2[0].num;
        if (eff_addr != assign_val->addr) {
            if (assign_val_flags & ASSIGN_VAL_IS_NUM) {
                if (!(reg_m_LRU(r_set, false).r->flags & REG_MODIFIED)) {
                    reg_allocator var = oper_get_reg_for_variable(eff_addr);
                    oper_reg_swap(var.r, assign_val);

                    var.r->addr = eff_addr;
                    var.r->flags |= REG_MODIFIED;
                } else {
                    reg_m_drop_addr(r_set, eff_addr);
                    assign_val->flags = REG_MODIFIED;
                    assign_val->addr = eff_addr;
                    reg_m_promote(r_set, assign_val->addr);
                }
            } else if (assign_val_flags & ASSIGN_VAL_STASH) {
                reg_m_drop_addr(r_set, eff_addr);
                assign_val->flags = REG_MODIFIED;
                assign_val->addr = eff_addr;
                reg_m_promote(r_set, assign_val->addr);
            } else {
                reg_allocator var = oper_get_reg_for_variable(eff_addr);
                oper_reg_swap(var.r, assign_val);

                var.r->addr = eff_addr;
                var.r->flags |= REG_MODIFIED;
            }
        } else {
            assign_val->flags |= REG_MODIFIED;
        }
    }
}

val oper_get_assign_val_1(expression_t const * const expr) {
    reg_set *r_set = get_reg_set();
    val assign_val = {
        .reg = NULL,
        .is_reg = true
    };

    if (!(expr->mask & LEFT_SYM1_NUM)) {
        if ((expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) && !(expr->mask & LEFT_SYM2_NUM)) {
            const bool left_sym_2_addr = expr->addr_mask & LEFT_SYM2_ADDR;
            const bool left_sym_2_const = !left_sym_2_addr && (expr->var_2[1].var->flags & SYMBOL_IS_CONSTANT);
            const bool left_sym_1_const = expr->var_1[1].var->flags & SYMBOL_IS_CONSTANT;

            if (left_sym_2_const && left_sym_1_const) {
                assign_val.is_reg = false;
                array_value *arr_val = expr->var_1[1].var->consts.arr_value;

                mpz_t idx;
                mpz_init_set(idx, expr->var_2[1].var->consts.value);
                mpz_sub_ui(idx, idx, expr->var_1[1].var->_add_info.start_idx);
                while (arr_val) {
                    if (mpz_cmp_ui(idx, arr_val->n) == 0) {
                        break;
                    }
                    arr_val = arr_val->next;
                }
                mpz_clear(idx);

                if (arr_val) {
                    mpz_init_set(assign_val.constant, arr_val->value);
                } else {
                    mpz_init_set_si(assign_val.constant, 0);
                }
            } else if (left_sym_2_const) {
                oper_store_array(expr->var_1[1].var->addr);
                reg_allocator var = oper_get_reg_for_variable(TEMP_ADDR_1);
                var.r->addr = TEMP_ADDR_1;

                mpz_t eff_addr;
                mpz_init_set_ui(eff_addr, expr->var_1[1].var->addr[0]);
                mpz_sub_ui(eff_addr, eff_addr, expr->var_1[1].var->_add_info.start_idx);
                mpz_add(eff_addr, eff_addr, expr->var_2[1].var->consts.value);
                stack_ptr_generate_from_mpz(eff_addr);
                mpz_clear(eff_addr);

                LOAD(var.r, &(r_set->stack_ptr));
                assign_val.reg = var.r;
            } else if (left_sym_1_const && left_sym_2_addr) { // during loops impl
            
            } else {
                if (left_sym_1_const) {
                    oper_flush_array_to_mem(expr->var_1[1].var);
                } else {
                    oper_store_array(expr->var_1[1].var->addr);
                }
                reg_allocator var = oper_get_reg_for_variable(TEMP_ADDR_1);
                var.r->addr = TEMP_ADDR_1;

                addr_t const var_idx_addr = (expr->addr_mask & LEFT_SYM2_ADDR) ? expr->var_2[1].addr : expr->var_2[1].var->addr[0];
                oper_set_stack_ptr_addr_arr(var_idx_addr,
                                            expr->var_1[1].var->addr[0],
                                            expr->var_1[1].var->_add_info.start_idx);

                reg_m_promote(r_set, TEMP_ADDR_1);
                LOAD(var.r, &(r_set->stack_ptr));

                assign_val.reg = var.r;
            }
        } else {
            const bool left_sym_1_const = expr->var_1[1].var->flags & SYMBOL_IS_CONSTANT;

            if (left_sym_1_const) {
                assign_val.is_reg = false;
                if (expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
                    array_value *arr_val = expr->var_1[1].var->consts.arr_value;
                    uint64_t idx = expr->var_2[1].num;
                    while (arr_val) {
                        if (arr_val->n == idx) {
                            break;
                        }
                        arr_val = arr_val->next;
                    }

                    if (arr_val) {
                        mpz_init_set(assign_val.constant, arr_val->value);
                    } else {
                        mpz_init_set_si(assign_val.constant, 0);
                    }
                } else {
                    mpz_init_set(assign_val.constant, expr->var_1[1].var->consts.value);
                }
            } else {
                addr_t const eff_addr = expr->var_1[1].var->addr[0] + (addr_t)expr->var_2[1].num;
                reg_allocator var = oper_get_reg_for_variable(eff_addr);

                if (!var.was_allocated) {
                    oper_load_variable_to_reg(var.r, eff_addr);
                }

                assign_val.reg = var.r;
            }
        }
    } else {
        assign_val.is_reg = false;
        mpz_init_set_si(assign_val.constant, expr->var_1[1].num);
    }

    return assign_val;
}

val oper_get_assign_val_2(expression_t const * const expr) {
    reg_set *r_set = get_reg_set();
    val assign_val = {
        .reg = NULL,
        .is_reg = true
    };

    if (!(expr->mask & RIGHT_SYM1_NUM)) {
        if ((expr->var_1[2].var->flags & SYMBOL_IS_ARRAY) && !(expr->mask & RIGHT_SYM2_NUM)) {
            const bool right_sym_2_addr = expr->addr_mask & RIGHT_SYM2_ADDR;
            const bool right_sym_2_const = !right_sym_2_addr && (expr->var_2[2].var->flags & SYMBOL_IS_CONSTANT);
            const bool right_sym_1_const = expr->var_1[2].var->flags & SYMBOL_IS_CONSTANT;

            if (right_sym_2_const && right_sym_1_const) {
                assign_val.is_reg = false;
                array_value *arr_val = expr->var_1[2].var->consts.arr_value;

                mpz_t idx;
                mpz_init_set(idx, expr->var_2[2].var->consts.value);
                mpz_sub_ui(idx, idx, expr->var_1[2].var->_add_info.start_idx);
                while (arr_val) {
                    if (mpz_cmp_ui(idx, arr_val->n) == 0) {
                        break;
                    }
                    arr_val = arr_val->next;
                }
                mpz_clear(idx);

                if (arr_val) {
                    mpz_init_set(assign_val.constant, arr_val->value);
                } else {
                    mpz_init_set_si(assign_val.constant, 0);
                }
            } else if (right_sym_2_const) {
                oper_store_array(expr->var_1[2].var->addr);
                reg_allocator var = oper_get_reg_for_variable(TEMP_ADDR_2);
                var.r->addr = TEMP_ADDR_2;

                mpz_t eff_addr;
                mpz_init_set_ui(eff_addr, expr->var_1[2].var->addr[0]);
                mpz_sub_ui(eff_addr, eff_addr, expr->var_1[2].var->_add_info.start_idx);
                mpz_add(eff_addr, eff_addr, expr->var_2[2].var->consts.value);
                stack_ptr_generate_from_mpz(eff_addr);
                mpz_clear(eff_addr);

                LOAD(var.r, &(r_set->stack_ptr));
                assign_val.reg = var.r;
            } else if (right_sym_1_const && right_sym_2_addr) { // during loops impl
            
            } else {
                if (right_sym_1_const) {
                    oper_flush_array_to_mem(expr->var_1[2].var);
                } else {
                    oper_store_array(expr->var_1[2].var->addr);
                }
                reg_allocator var = oper_get_reg_for_variable(TEMP_ADDR_2);
                var.r->addr = TEMP_ADDR_2;

                addr_t const var_idx_addr = (expr->addr_mask & RIGHT_SYM2_ADDR) ? expr->var_2[2].addr : expr->var_2[2].var->addr[0];
                oper_set_stack_ptr_addr_arr(var_idx_addr,
                                            expr->var_1[2].var->addr[0],
                                            expr->var_1[2].var->_add_info.start_idx);

                reg_m_promote(r_set, TEMP_ADDR_2);
                LOAD(var.r, &(r_set->stack_ptr));

                assign_val.reg = var.r;
            }
        } else {
            const bool right_sym_1_const = expr->var_1[2].var->flags & SYMBOL_IS_CONSTANT;

            if (right_sym_1_const) {
                assign_val.is_reg = false;
                if (expr->var_1[2].var->flags & SYMBOL_IS_ARRAY) {
                    array_value *arr_val = expr->var_1[2].var->consts.arr_value;
                    uint64_t idx = expr->var_2[2].num;
                    while (arr_val) {
                        if (arr_val->n == idx) {
                            break;
                        }
                        arr_val = arr_val->next;
                    }

                    if (arr_val) {
                        mpz_init_set(assign_val.constant, arr_val->value);
                    } else {
                        mpz_init_set_si(assign_val.constant, 0);
                    }
                } else {
                    mpz_init_set(assign_val.constant, expr->var_1[2].var->consts.value);
                }
            } else {
                addr_t const eff_addr = expr->var_1[2].var->addr[0] + (addr_t)expr->var_2[2].num;
                reg_allocator var = oper_get_reg_for_variable(eff_addr);

                if (!var.was_allocated) {
                    oper_load_variable_to_reg(var.r, eff_addr);
                }

                assign_val.reg = var.r;
            }
        }
    } else {
        assign_val.is_reg = false;
        mpz_init_set_si(assign_val.constant, expr->var_1[2].num);
    }

    return assign_val;
}
