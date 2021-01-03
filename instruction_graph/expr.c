#include "i_graph.h"
#include "expr_checker.h"
#include "../parser_func/getters.h"
#include "generators/stack_generator.h"
#include "generators/val_generator.h"
#include "std_oper/std_oper.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_EXPR(expression_t *expr) {
    eval_check_0(expr);
    eval_check_1(expr);

    if (expr->type != expr_VALUE) {
        eval_check_2(expr);
    }

    add_to_list(expr, i_EXPR);
}

static void eval_expr_VALUE(expression_t const * const expr, FILE *file) {
    reg_set *r_set = get_reg_set();

    reg *assign_val = NULL;
    if (!(expr->mask & LEFT_SYM1_NUM)) {
        if (expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
            if (!(expr->mask & LEFT_SYM2_NUM)) {
                oper_store_array(expr->var_1[1].var->addr, file);

                reg_allocator var = oper_get_reg_for_variable(TEMP_ADDR_1, file);

                addr_t const var_idx_addr = (expr->addr_mask & LEFT_SYM1_ADDR) ? expr->var_2[1].addr : expr->var_2[1].var->addr[0];
                oper_set_stack_ptr_addr_arr(var_idx_addr,
                                            expr->var_1[1].var->addr[0],
                                            expr->var_1[1].var->_add_info.start_idx,
                                            file);
                fprintf(file, "LOAD %c %c\n", var.r->id, r_set->stack_ptr.id);

                assign_val = var.r;
            } else {
                addr_t const eff_addr = expr->var_1[1].var->addr[0] + (addr_t)expr->var_2[1].num;
                reg_allocator var = oper_get_reg_for_variable(eff_addr, file);

                if (!var.was_allocated) {
                    oper_load_variable_to_reg(var.r, eff_addr, file);
                }

                assign_val = var.r;
            }
        } else {
            addr_t const eff_addr = expr->var_1[1].var->addr[0];
            reg_allocator var = oper_get_reg_for_variable(eff_addr, file);

            if (!var.was_allocated) {
                oper_load_variable_to_reg(var.r, eff_addr, file);
            }

            assign_val = var.r;
        }
    } else {
        assign_val = val_generate(expr->var_1[1].num, file);
    }

    if (expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) {
        if (!(expr->mask & ASSIGN_SYM2_NUM)) {
            addr_t const var_idx_addr = (expr->addr_mask & ASSIGN_SYM2_ADDR) ? expr->var_2[0].addr : expr->var_2[0].var->addr[0];
            oper_set_stack_ptr_addr_arr(var_idx_addr,
                                        expr->var_1[0].var->addr[0],
                                        expr->var_1[0].var->_add_info.start_idx,
                                        file);
            fprintf(file, "STORE %c %c\n", assign_val->id, r_set->stack_ptr.id);
        } else {
            addr_t const eff_addr = expr->var_1[0].var->addr[0] + (addr_t)expr->var_2[0].num;
            if (eff_addr != assign_val->addr) {
                if (!(expr->mask & LEFT_SYM1_NUM)) {
                    reg_allocator var = oper_get_reg_for_variable(eff_addr, file);
                    oper_reg_swap(var.r, assign_val, file);

                    var.r->addr = eff_addr;
                    var.r->flags |= REG_MODIFIED;
                } else {
                    reg_m_drop_addr(r_set, eff_addr);
                    assign_val->flags = REG_MODIFIED;
                    assign_val->addr = eff_addr;
                }
            }
        }
    } else {
        addr_t const eff_addr = expr->var_1[0].var->addr[0];
        if (eff_addr != assign_val->addr) {
            if (!(expr->mask & LEFT_SYM1_NUM)) {
                reg_allocator var = oper_get_reg_for_variable(eff_addr, file);
                oper_reg_swap(var.r, assign_val, file);

                var.r->addr = eff_addr;
                var.r->flags |= REG_MODIFIED;
            } else {
                reg_m_drop_addr(r_set, eff_addr);
                assign_val->flags = REG_MODIFIED;
                assign_val->addr = eff_addr;
            }
        }
    }

    reg_m_drop_addr(r_set, TEMP_ADDR_1);
}

void eval_EXPR(i_graph **i_current, FILE *file) {
    expression_t const * const expr_curr = (*i_current)->payload;

    switch (expr_curr->type) {
        case expr_VALUE:
            eval_expr_VALUE(expr_curr, file);
            break;
        case expr_ADD:
            break;
        case expr_SUB:
            break;
        case expr_MUL:
            break;
        case expr_DIV:
            break;
        case expr_MOD:
            break;
        default:
            fprintf(stderr, "[EXPR]: Wrong type of expression!\n");
            exit(EXIT_FAILURE);
    }
}
