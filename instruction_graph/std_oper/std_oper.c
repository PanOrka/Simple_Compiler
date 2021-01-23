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
    reg *s_idx = val_generate(start_idx);
    stack_ptr_generate(arr_addr);
    ADD(stack_ptr, var_idx);
    SUB(stack_ptr, s_idx);
    stack_ptr_clear();
}

static void dec_val_gen(reg *stack_ptr, addr_t arr_addr, reg *var_idx, num_t eff_addr) {
    stack_ptr_generate(arr_addr);
    ADD(stack_ptr, var_idx);
    for (num_t i=eff_addr; i<1; ++i) {
        DEC(stack_ptr);
    }
    stack_ptr_clear();
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
    if (r1 != r2) {
        RESET(r1);
        ADD(r1, r2);
    }
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

void oper_flush_array_to_mem(symbol *arr) {
    reg_set *r_set = get_reg_set();
    array_value *arr_val = arr->consts.arr_value;
    while (arr_val) {
        if (arr_val->is_constant && !(arr_val->is_in_memory)) {
            reg *val_reg = val_generate_from_mpz(arr_val->value);
            stack_ptr_generate(arr->addr[0] + arr_val->n);
            STORE(val_reg, &(r_set->stack_ptr));
            arr_val->is_in_memory = true;
        }
        arr_val = arr_val->next;
    }
}

array_value * oper_arr_val_find(array_value *arr_val, uint64_t idx_ui) {
    while (arr_val) {
        if (idx_ui == arr_val->n) {
            break;
        }
        arr_val = arr_val->next;
    }

    return arr_val;
}

void oper_arr_val_add(symbol *arr, array_value new_arr_val) {
    array_value *arr_val = arr->consts.arr_value;
    new_arr_val.next = NULL;

    array_value *new_node = malloc(sizeof(array_value));
    *new_node = new_arr_val;
    if (arr_val) {
        while (arr_val->next) {
            arr_val = arr_val->next;
        }
        arr_val->next = new_node;
    } else {
        arr->consts.arr_value = new_node;
    }
}

void oper_arr_set_non_constant(symbol *arr) {
    array_value *arr_val = arr->consts.arr_value;
    while (arr_val) {
        mpz_set_si(arr_val->value, 0);
        arr_val->is_in_memory = true;
        arr_val->is_constant = false;
        arr_val = arr_val->next;
    }
}

static void oper_set_reg(reg *assign_val, addr_t eff_addr, uint8_t assign_val_flags) {
    reg_set *r_set = get_reg_set();
    if (eff_addr != assign_val->addr) {
        if (assign_val_flags & ASSIGN_VAL_STASH) {
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

#include "../i_level.h"
uint64_t generate_from_reset_cost(mpz_t target_val);

void oper_set_assign_val_0(expression_t const * const expr,
                           val assign_val,
                           uint8_t assign_val_flags)
{
    const bool i_level_empty = i_level_is_empty_eval();
    bool generate_cost_too_big = false;
    reg_set *r_set = get_reg_set();
    symbol_table *s_table = get_symbol_table();
    if (!assign_val.is_reg && (generate_cost_too_big = (generate_from_reset_cost(assign_val.constant) >= 40))) {
        assign_val.reg = val_generate_from_mpz(assign_val.constant);
        assign_val.is_reg = true;
    }

    symbol *var_1 = symbol_table_find_by_idx(s_table, expr->var_1[0].sym_idx);
    if ((var_1->flags & SYMBOL_IS_ARRAY) && !(expr->mask & ASSIGN_SYM2_NUM)) {
        const bool assign_sym_2_addr = expr->addr_mask & ASSIGN_SYM2_ADDR;

        symbol *var_2 = NULL;
        if (!assign_sym_2_addr) {
            var_2 = symbol_table_find_by_idx(s_table, expr->var_2[0].sym_idx);
        }
        const bool assign_sym_2_const = !assign_sym_2_addr && (var_2->flags & SYMBOL_IS_CONSTANT);
        const bool assign_sym_1_const = (var_1->flags & SYMBOL_IS_CONSTANT) && i_level_empty;

        if (assign_sym_2_const && assign_sym_1_const) {
            array_value *arr_val = var_1->consts.arr_value;

            mpz_t idx;
            mpz_init_set(idx, var_2->consts.value);
            mpz_sub_ui(idx, idx, var_1->_add_info.start_idx);

            const uint64_t idx_ui = mpz_get_ui(idx);
            mpz_clear(idx);
            arr_val = oper_arr_val_find(arr_val, idx_ui);

            addr_t const eff_addr = var_1->addr[0] + idx_ui;

            if (arr_val) {
                if (assign_val.is_reg) {
                    arr_val->is_constant = false;
                    arr_val->is_in_memory = false;
                    mpz_set_si(arr_val->value, 0);

                    oper_set_reg(assign_val.reg, eff_addr, assign_val_flags);
                } else {
                    reg_m_drop_addr(r_set, eff_addr);
                    arr_val->is_constant = true;
                    arr_val->is_in_memory = false;
                    mpz_set(arr_val->value, assign_val.constant);
                }
            } else {
                if (assign_val.is_reg) {
                    array_value new_arr_val = {
                        .is_constant = false,
                        .is_in_memory = false,
                        .n = idx_ui
                    };
                    mpz_init(new_arr_val.value);
                    oper_arr_val_add(var_1, new_arr_val);
                    oper_set_reg(assign_val.reg, eff_addr, assign_val_flags);
                } else {
                    reg_m_drop_addr(r_set, eff_addr);
                    array_value new_arr_val = {
                        .is_constant = true,
                        .is_in_memory = false,
                        .n = idx_ui
                    };
                    mpz_init_set(new_arr_val.value, assign_val.constant);
                    oper_arr_val_add(var_1, new_arr_val);
                }
            }
        } else if (assign_sym_2_const) {
            mpz_t eff_addr;
            mpz_init_set_ui(eff_addr, var_1->addr[0]);
            mpz_sub_ui(eff_addr, eff_addr, var_1->_add_info.start_idx);
            mpz_add(eff_addr, eff_addr, var_2->consts.value);

            const addr_t eff_addr_ui = mpz_get_ui(eff_addr);
            mpz_clear(eff_addr);
            if (assign_val.is_reg) {
                oper_set_reg(assign_val.reg, eff_addr_ui, assign_val_flags);
            } else {
                reg *val_reg = val_generate_from_mpz(assign_val.constant);
                oper_set_reg(val_reg, eff_addr_ui, assign_val_flags);
            }
        } else {
            if (assign_sym_1_const) {
                oper_flush_array_to_mem(var_1);
                oper_arr_set_non_constant(var_1);
            }
            
            oper_store_array(var_1->addr);

            reg *store_reg;
            if (assign_val.is_reg) {
                store_reg = assign_val.reg;
                if (generate_cost_too_big) {
                    store_reg->addr = TEMP_ADDR_1;
                }
            } else {
                store_reg = val_generate_from_mpz(assign_val.constant);
                store_reg->addr = TEMP_ADDR_1;
            }

            addr_t const var_idx_addr = (expr->addr_mask & ASSIGN_SYM2_ADDR) ? expr->var_2[0].addr : var_2->addr[0];
            oper_set_stack_ptr_addr_arr(var_idx_addr,
                                        var_1->addr[0],
                                        var_1->_add_info.start_idx);

            oper_drop_array(var_1->addr);

            STORE(store_reg, &(r_set->stack_ptr));
            if (assign_val_flags & ASSIGN_VAL_STASH) {
                reg_m_drop_addr(r_set, store_reg->addr);
            }
        }
    } else {
        const bool assign_sym_1_const = (var_1->flags & SYMBOL_IS_CONSTANT) && i_level_empty;
        const uint64_t idx = expr->var_2[0].num;
        addr_t const eff_addr = var_1->addr[0] + idx;

        if (assign_sym_1_const) {
            if (var_1->flags & SYMBOL_IS_ARRAY) {
                array_value *arr_val = var_1->consts.arr_value;
                arr_val = oper_arr_val_find(arr_val, idx);

                if (arr_val) {
                    if (assign_val.is_reg) {
                        arr_val->is_in_memory = false;
                        arr_val->is_constant = false;
                        mpz_set_si(arr_val->value, 0);
                        oper_set_reg(assign_val.reg, eff_addr, assign_val_flags);
                    } else {
                        reg_m_drop_addr(r_set, eff_addr);
                        arr_val->is_in_memory = false;
                        arr_val->is_constant = true;
                        mpz_set(arr_val->value, assign_val.constant);
                    }
                } else {
                    if (assign_val.is_reg) {
                        array_value new_arr_val = {
                            .is_constant = false,
                            .is_in_memory = false,
                            .n = idx
                        };
                        mpz_init(new_arr_val.value);
                        oper_arr_val_add(var_1, new_arr_val);
                        oper_set_reg(assign_val.reg, eff_addr, assign_val_flags);
                    } else {
                        reg_m_drop_addr(r_set, eff_addr);
                        array_value new_arr_val = {
                            .is_constant = true,
                            .is_in_memory = false,
                            .n = idx
                        };
                        mpz_init_set(new_arr_val.value, assign_val.constant);
                        oper_arr_val_add(var_1, new_arr_val);
                    }
                }
            } else {
                if (assign_val.is_reg) {
                    var_1->flags &= ~SYMBOL_IS_CONSTANT;
                    mpz_set_si(var_1->consts.value, 0);
                    oper_set_reg(assign_val.reg, eff_addr, assign_val_flags);
                } else {
                    reg_m_drop_addr(r_set, eff_addr);
                    var_1->flags |= SYMBOL_IS_CONSTANT;
                    mpz_set(var_1->consts.value, assign_val.constant);
                    var_1->symbol_in_memory = false;
                }
            }
        } else {
            if (assign_val.is_reg) {
                oper_set_reg(assign_val.reg, eff_addr, assign_val_flags);
            } else if (!i_level_empty) {
                reg *val_reg = val_generate_from_mpz(assign_val.constant);
                oper_set_reg(val_reg, eff_addr, assign_val_flags);
            } else {
                reg_m_drop_addr(r_set, eff_addr);
                var_1->flags |= SYMBOL_IS_CONSTANT;
                mpz_set(var_1->consts.value, assign_val.constant);
                var_1->symbol_in_memory = false;
            }
        }
    }
}

static reg * load_sym_reg_from_const(symbol *var_1, symbol *var_2) {
    mpz_t eff_addr;
    mpz_init_set_ui(eff_addr, var_1->addr[0]);
    mpz_sub_ui(eff_addr, eff_addr, var_1->_add_info.start_idx);
    mpz_add(eff_addr, eff_addr, var_2->consts.value);

    const addr_t eff_addr_ui = mpz_get_ui(eff_addr);
    mpz_clear(eff_addr);

    reg_allocator var = oper_get_reg_for_variable(eff_addr_ui);

    if (!var.was_allocated) {
        oper_load_variable_to_reg(var.r, eff_addr_ui);
    }

    return var.r;
}

static reg * load_sym_reg_from_num(symbol *var_1, addr_t num) {
    addr_t const eff_addr = var_1->addr[0] + num;
    reg_allocator var = oper_get_reg_for_variable(eff_addr);

    if (!var.was_allocated) {
        oper_load_variable_to_reg(var.r, eff_addr);
    }

    return var.r;
}

val oper_get_assign_val_1(expression_t const * const expr) {
    reg_set *r_set = get_reg_set();
    symbol_table *s_table = get_symbol_table();
    val assign_val = {
        .reg = NULL,
        .is_reg = true
    };

    const bool left_sym_1_addr = expr->addr_mask & LEFT_SYM1_ADDR;
    if (left_sym_1_addr) {
        reg_allocator var = oper_get_reg_for_variable(expr->var_1[1].addr);
        if (!var.was_allocated) {
            oper_load_variable_to_reg(var.r, expr->var_1[1].addr);
        }

        assign_val.reg = var.r;
    } else if (!(expr->mask & LEFT_SYM1_NUM)) {
        symbol *var_1 = symbol_table_find_by_idx(s_table, expr->var_1[1].sym_idx);
        if ((var_1->flags & SYMBOL_IS_ARRAY) && !(expr->mask & LEFT_SYM2_NUM)) {
            const bool left_sym_2_addr = expr->addr_mask & LEFT_SYM2_ADDR;

            symbol *var_2 = NULL;
            if (!left_sym_2_addr) {
                var_2 = symbol_table_find_by_idx(s_table, expr->var_2[1].sym_idx);
            }
            const bool left_sym_2_const = !left_sym_2_addr && (var_2->flags & SYMBOL_IS_CONSTANT);
            const bool left_sym_1_const = var_1->flags & SYMBOL_IS_CONSTANT;

            if (left_sym_2_const && left_sym_1_const) {
                assign_val.is_reg = false;
                array_value *arr_val = var_1->consts.arr_value;

                mpz_t idx;
                mpz_init_set(idx, var_2->consts.value);
                mpz_sub_ui(idx, idx, var_1->_add_info.start_idx);

                const uint64_t idx_ui = mpz_get_ui(idx);
                mpz_clear(idx);
                arr_val = oper_arr_val_find(arr_val, idx_ui);

                if (arr_val) {
                    if (arr_val->is_constant) {
                        mpz_init_set(assign_val.constant, arr_val->value);
                    } else {
                        assign_val.is_reg = true;
                        assign_val.reg = load_sym_reg_from_const(var_1, var_2);
                    }
                } else {
                    assign_val.is_reg = true;
                    assign_val.reg = load_sym_reg_from_const(var_1, var_2);
                }
            } else if (left_sym_2_const) {
                assign_val.reg = load_sym_reg_from_const(var_1, var_2);
            } else {
                if (left_sym_1_const) {
                    oper_flush_array_to_mem(var_1);
                }
                oper_store_array(var_1->addr);
                reg_allocator var = oper_get_reg_for_variable(TEMP_ADDR_1);
                var.r->addr = TEMP_ADDR_1;

                addr_t const var_idx_addr = (expr->addr_mask & LEFT_SYM2_ADDR) ? expr->var_2[1].addr : var_2->addr[0];
                oper_set_stack_ptr_addr_arr(var_idx_addr,
                                            var_1->addr[0],
                                            var_1->_add_info.start_idx);

                reg_m_promote(r_set, TEMP_ADDR_1);
                LOAD(var.r, &(r_set->stack_ptr));

                assign_val.reg = var.r;
            }
        } else {
            const bool left_sym_1_const = var_1->flags & SYMBOL_IS_CONSTANT;

            if (left_sym_1_const) {
                assign_val.is_reg = false;
                if (var_1->flags & SYMBOL_IS_ARRAY) {
                    array_value *arr_val = var_1->consts.arr_value;
                    const uint64_t idx = expr->var_2[1].num;
                    arr_val = oper_arr_val_find(arr_val, idx);

                    if (arr_val) {
                        if (arr_val->is_constant) {
                            mpz_init_set(assign_val.constant, arr_val->value);
                        } else {
                            assign_val.is_reg = true;
                            assign_val.reg = load_sym_reg_from_num(var_1, idx);
                        }
                    } else {
                        assign_val.is_reg = true;
                        assign_val.reg = load_sym_reg_from_num(var_1, idx);
                    }
                } else {
                    mpz_init_set(assign_val.constant, var_1->consts.value);
                }
            } else {
                assign_val.reg = load_sym_reg_from_num(var_1, expr->var_2[1].num);
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
    symbol_table *s_table = get_symbol_table();
    val assign_val = {
        .reg = NULL,
        .is_reg = true
    };

    const bool right_sym_1_addr = expr->addr_mask & RIGHT_SYM1_ADDR;
    if (right_sym_1_addr) {
        reg_allocator var = oper_get_reg_for_variable(expr->var_1[2].addr);
        if (!var.was_allocated) {
            oper_load_variable_to_reg(var.r, expr->var_1[2].addr);
        }

        assign_val.reg = var.r;
    } else if (!(expr->mask & RIGHT_SYM1_NUM)) {
        symbol *var_1 = symbol_table_find_by_idx(s_table, expr->var_1[2].sym_idx);
        if ((var_1->flags & SYMBOL_IS_ARRAY) && !(expr->mask & RIGHT_SYM2_NUM)) {
            const bool right_sym_2_addr = expr->addr_mask & RIGHT_SYM2_ADDR;

            symbol *var_2 = NULL;
            if (!right_sym_2_addr) {
                var_2 = symbol_table_find_by_idx(s_table, expr->var_2[2].sym_idx);
            }
            const bool right_sym_2_const = !right_sym_2_addr && (var_2->flags & SYMBOL_IS_CONSTANT);
            const bool right_sym_1_const = var_1->flags & SYMBOL_IS_CONSTANT;

            if (right_sym_2_const && right_sym_1_const) {
                assign_val.is_reg = false;
                array_value *arr_val = var_1->consts.arr_value;

                mpz_t idx;
                mpz_init_set(idx, var_2->consts.value);
                mpz_sub_ui(idx, idx, var_1->_add_info.start_idx);

                const uint64_t idx_ui = mpz_get_ui(idx);
                mpz_clear(idx);
                arr_val = oper_arr_val_find(arr_val, idx_ui);

                if (arr_val) {
                    if (arr_val->is_constant) {
                        mpz_init_set(assign_val.constant, arr_val->value);
                    } else {
                        assign_val.is_reg = true;
                        assign_val.reg = load_sym_reg_from_const(var_1, var_2);
                    }
                } else {
                    assign_val.is_reg = true;
                    assign_val.reg = load_sym_reg_from_const(var_1, var_2);
                }
            } else if (right_sym_2_const) {
                assign_val.reg = load_sym_reg_from_const(var_1, var_2);
            } else {
                if (right_sym_1_const) {
                    oper_flush_array_to_mem(var_1);
                }
                oper_store_array(var_1->addr);
                reg_allocator var = oper_get_reg_for_variable(TEMP_ADDR_2);
                var.r->addr = TEMP_ADDR_2;

                addr_t const var_idx_addr = (expr->addr_mask & RIGHT_SYM2_ADDR) ? expr->var_2[2].addr : var_2->addr[0];
                oper_set_stack_ptr_addr_arr(var_idx_addr,
                                            var_1->addr[0],
                                            var_1->_add_info.start_idx);

                reg_m_promote(r_set, TEMP_ADDR_2);
                LOAD(var.r, &(r_set->stack_ptr));

                assign_val.reg = var.r;
            }
        } else {
            const bool right_sym_1_const = var_1->flags & SYMBOL_IS_CONSTANT;

            if (right_sym_1_const) {
                assign_val.is_reg = false;
                if (var_1->flags & SYMBOL_IS_ARRAY) {
                    array_value *arr_val = var_1->consts.arr_value;
                    const uint64_t idx = expr->var_2[2].num;
                    arr_val = oper_arr_val_find(arr_val, idx);

                    if (arr_val) {
                        if (arr_val->is_constant) {
                            mpz_init_set(assign_val.constant, arr_val->value);
                        } else {
                            assign_val.is_reg = true;
                            assign_val.reg = load_sym_reg_from_num(var_1, idx);
                        }
                    } else {
                        assign_val.is_reg = true;
                        assign_val.reg = load_sym_reg_from_num(var_1, idx);
                    }
                } else {
                    mpz_init_set(assign_val.constant, var_1->consts.value);
                }
            } else {
                assign_val.reg = load_sym_reg_from_num(var_1, expr->var_2[2].num);
            }
        }
    } else {
        assign_val.is_reg = false;
        mpz_init_set_si(assign_val.constant, expr->var_1[2].num);
    }

    return assign_val;
}

void oper_regs_store_drop() {
    reg_set *r_set = get_reg_set();
    for (int32_t i=0; i<REG_SIZE; ++i) {
        reg *clear_reg = r_set->r[i];
        if (clear_reg->flags & REG_MODIFIED) { // First register is always stashed
            stack_ptr_generate(clear_reg->addr);
            STORE(clear_reg, &(r_set->stack_ptr));
            clear_reg->flags &= ~REG_MODIFIED;
        }
        clear_reg->addr = ADDR_UNDEF;
        clear_reg->flags = REG_NO_FLAGS;
    }
}

void oper_store_reg(reg *r) {
    reg_set *r_set = get_reg_set();

    if (r->flags & REG_MODIFIED) {
        stack_ptr_generate(r->addr);
        STORE(r, &(r_set->stack_ptr));
        r->flags &= ~REG_MODIFIED;
    }
}
