#include "cond.h"

#include <stdio.h>

bool cond_val_from_const(mpz_t src_1, mpz_t src_2, expr_type type) {
    int res = mpz_cmp(src_1, src_2);
    switch (type) {
        case cond_IS_EQUAL:
            return res == 0;
            break;
        case cond_IS_N_EQUAL:
            return res != 0;
            break;
        case cond_LESS:
            return res < 0;
            break;
        case cond_GREATER:
            return res > 0;
            break;
        case cond_LESS_EQ:
            return res <= 0;
            break;
        case cond_GREATER_EQ:
            return res >= 0;
            break;
        default:
            fprintf(stderr, "[COND]: Wrong value of cond_type!\n");
            exit(EXIT_FAILURE);
    }
}

#include "../generators/val_generator.h"
#include "../generators/stack_generator.h"
#include "../instructions/asm_fprintf.h"
#include "../../parser_func/getters.h"

reg * cond_val_from_vals(val src_1, val src_2, expr_type type) {
    reg_set *r_set = get_reg_set();

    reg *x = src_1.is_reg ? src_1.reg : val_generate_from_mpz(src_1.constant);
    reg *y = src_2.is_reg ? src_2.reg : val_generate_from_mpz(src_2.constant);

    switch (type) {
        case cond_IS_EQUAL: // x == y
            if (y->flags & REG_MODIFIED) {
                stack_ptr_generate(y->addr);
                STORE(y, &(r_set->stack_ptr));
                y->flags &= ~REG_MODIFIED;
            }
            reg *temp = oper_get_reg_for_variable(TEMP_ADDR_3).r;
            oper_reg_swap(temp, x);
            SUB(temp, y);
            SUB(y, x);
            ADD(temp, y);
            reg_m_drop_addr(r_set, y->addr);

            JUMP_i_idx(2);
            INC(temp);
            JZERO_i_idx(temp, -1);
            reg_m_promote(r_set, temp);

            return temp;
            break;
        case cond_IS_N_EQUAL:
            if (y->flags & REG_MODIFIED) {
                stack_ptr_generate(y->addr);
                STORE(y, &(r_set->stack_ptr));
                y->flags &= ~REG_MODIFIED;
            }
            reg *temp = oper_get_reg_for_variable(TEMP_ADDR_3).r;
            oper_reg_swap(temp, x);
            SUB(temp, y);
            SUB(y, x);
            ADD(temp, y);
            reg_m_drop_addr(r_set, y->addr);
            reg_m_promote(r_set, temp);

            return temp;
            break;
        case cond_LESS:
            if (y->flags & REG_MODIFIED) {
                stack_ptr_generate(y->addr);
                STORE(y, &(r_set->stack_ptr));
                y->flags &= ~REG_MODIFIED;
            }
            SUB(y, x);
            y->addr = TEMP_ADDR_2;
            reg_m_promote(r_set, y->addr);

            return y;
            break;
        case cond_GREATER:
            if (x->flags & REG_MODIFIED) {
                stack_ptr_generate(x->addr);
                STORE(x, &(r_set->stack_ptr));
                x->flags &= ~REG_MODIFIED;
            }
            SUB(x, y);
            x->addr = TEMP_ADDR_1;
            reg_m_promote(r_set, x->addr);

            return x;
            break;
        case cond_LESS_EQ: // x <= y
            if (y->flags & REG_MODIFIED) {
                stack_ptr_generate(y->addr);
                STORE(y, &(r_set->stack_ptr));
                y->flags &= ~REG_MODIFIED;
            }
            INC(y);
            SUB(y, x);
            y->addr = TEMP_ADDR_2;
            reg_m_promote(r_set, y->addr);

            return y;
            break;
        case cond_GREATER_EQ: // x >= y
            if (x->flags & REG_MODIFIED) {
                stack_ptr_generate(x->addr);
                STORE(x, &(r_set->stack_ptr));
                x->flags &= ~REG_MODIFIED;
            }
            INC(x);
            SUB(x, y);
            x->addr = TEMP_ADDR_1;
            reg_m_promote(r_set, x->addr);

            return x;
            break;
        default:
            fprintf(stderr, "[COND]: Wrong value of cond_type!\n");
            exit(EXIT_FAILURE);
    }
}
