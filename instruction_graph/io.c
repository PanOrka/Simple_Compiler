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
    expression_t const * const expr_curr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    if ((expr_curr->var_1[0].var->flags & SYMBOL_IS_ARRAY) && !(expr_curr->mask & ASSIGN_SYM2_NUM)) {
        oper_store_array(expr_curr->var_1[0].var->addr);
        oper_drop_array(expr_curr->var_1[0].var->addr);

        addr_t const var_idx_addr = (expr_curr->addr_mask & ASSIGN_SYM2_ADDR) ? expr_curr->var_2[0].addr : expr_curr->var_2[0].var->addr[0];
        oper_set_stack_ptr_addr_arr(var_idx_addr,
                                    expr_curr->var_1[0].var->addr[0],
                                    expr_curr->var_1[0].var->_add_info.start_idx);
    } else {
        addr_t const eff_addr = expr_curr->var_1[0].var->addr[0] + (addr_t)expr_curr->var_2[0].num;
        stack_ptr_generate(eff_addr);
        reg_m_drop_addr(r_set, eff_addr);
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

void eval_WRITE(i_graph **i_current) {
    expression_t const * const expr_curr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    if (!(expr_curr->mask & LEFT_SYM1_NUM)) {
        if ((expr_curr->var_1[1].var->flags & SYMBOL_IS_ARRAY) && !(expr_curr->mask & LEFT_SYM2_NUM)) {
            oper_store_array(expr_curr->var_1[1].var->addr);

            addr_t const var_idx_addr = (expr_curr->addr_mask & LEFT_SYM2_ADDR) ? expr_curr->var_2[1].addr : expr_curr->var_2[1].var->addr[0];
            oper_set_stack_ptr_addr_arr(var_idx_addr,
                                        expr_curr->var_1[1].var->addr[0],
                                        expr_curr->var_1[1].var->_add_info.start_idx);
        } else {
            addr_t const eff_addr = expr_curr->var_1[1].var->addr[0] + (addr_t)expr_curr->var_2[1].num;
            stack_ptr_generate(eff_addr);
            reg_allocator reg_alloc = reg_m_get(r_set, eff_addr, false);
            if (reg_alloc.was_allocated && (reg_alloc.r->flags & REG_MODIFIED)) {
                STORE(reg_alloc.r, &(r_set->stack_ptr));
                reg_alloc.r->flags &= ~REG_MODIFIED;
            }
        }
    } else {
        addr_t const temp_write_addr = expr_curr->var_1[0].addr;
        reg *write_reg = val_generate(expr_curr->var_1[1].num);
        stack_ptr_generate(temp_write_addr);
        STORE(write_reg, &(r_set->stack_ptr));
    }

    PUT(&(r_set->stack_ptr));
}
