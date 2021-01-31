#include "i_graph.h"
#include "expr_checker.h"
#include "../parser_func/getters.h"
#include "generators/stack_generator.h"
#include "generators/val_generator.h"
#include "std_oper/std_oper.h"
#include "arithmetic/arithmetic.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_EXPR(expression_t *expr) {
    eval_check_0(expr);
    eval_check_1(expr);

    if (expr->type != expr_VALUE) {
        eval_check_2(expr);
    }

    add_to_list(expr, i_EXPR);
}


static void expr_get_var_mask(expression_t const * const expr, bool check_second, bool mask_assign[2]);


static void eval_expr_VALUE(expression_t const * const expr) {
    reg_set *r_set = get_reg_set();

    bool mask_assign[2];
    expr_get_var_mask(expr, false, mask_assign);

    val assign_val = oper_get_assign_val_1(expr);
    if (mask_assign[0] || (assign_val.is_reg && (assign_val.reg->addr == TEMP_ADDR_1))) {
        oper_set_assign_val_0(expr, assign_val, ASSIGN_VAL_STASH);
    } else {
        oper_set_assign_val_0(expr, assign_val, ASSIGN_VAL_NO_FLAGS);
    }

    if (!assign_val.is_reg) {
        mpz_clear(assign_val.constant);
    }

    reg_m_drop_addr(r_set, TEMP_ADDR_1);
}

static void num_add(mpz_t dest, mpz_t src_1, mpz_t src_2) {
    mpz_add(dest, src_1, src_2);
}

static void num_sub(mpz_t dest, mpz_t src_1, mpz_t src_2) {
    mpz_sub(dest, src_1, src_2);
    if (mpz_cmp_si(dest, 0) < 0) {
        mpz_set_si(dest, 0);
    }
}

static void num_mul(mpz_t dest, mpz_t src_1, mpz_t src_2) {
    mpz_mul(dest, src_1, src_2);
}

static void num_div(mpz_t dest, mpz_t src_1, mpz_t src_2) {
    if (mpz_cmp_ui(src_2, 0) == 0) {
        mpz_set_si(dest, 0);
    } else {
        mpz_tdiv_q(dest, src_1, src_2);
    }
}

static void num_mod(mpz_t dest, mpz_t src_1, mpz_t src_2) {
    if (mpz_cmp_ui(src_2, 0) == 0) {
        mpz_set_si(dest, 0);
    } else {
        mpz_tdiv_r(dest, src_1, src_2);
    }
}


typedef struct {
    void (*func_num) (mpz_t dest, mpz_t src_1, mpz_t src_2);
    val (*func_reg) (val x, val y, uint8_t *flags, bool mask_assign[2]);
} arithmetic_func;


static void eval_expr_ARITHMETIC(expression_t const * const expr, arithmetic_func func) {
    reg_set *r_set = get_reg_set();
    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);

    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    if (!(assign_val_1.is_reg || assign_val_2.is_reg)) { // both constants
        func.func_num(assign_val_1.constant, assign_val_1.constant, assign_val_2.constant);
        mpz_clear(assign_val_2.constant);
        oper_set_assign_val_0(expr, assign_val_1, ASSIGN_VAL_NO_FLAGS);
        mpz_clear(assign_val_1.constant);
    } else {
        bool mask_assign[2];
        expr_get_var_mask(expr, true, mask_assign);
        uint8_t assign_val_flags = ASSIGN_VAL_INVALID_FLAG;
        val new_val = func.func_reg(assign_val_1, assign_val_2, &assign_val_flags, mask_assign);
        if (assign_val_flags == ASSIGN_VAL_INVALID_FLAG) {
            fprintf(stderr, "[EXPR]: Invalid assign_val_flag!\n");
            exit(EXIT_FAILURE);
        }

        if (new_val.is_reg) {
            reg_m_promote(r_set, new_val.reg->addr);
        }

        oper_set_assign_val_0(expr, new_val, assign_val_flags);
        if (!new_val.is_reg) {
            mpz_clear(new_val.constant);
        }
    }

    // FOR NOW it's glued up with TODO in std_oper get_assign functions
    // Could be optimized:
    // - save or stash VAL_GEN_ADDR register depending on situation with other variables
    // - definetly let TEMP_ADDR_1 or TEMP_ADDR_2 to be sum
    // - reuse code - use it for abelowe operacje
    // - consider var_1 := var_1 (oper) var_2 for optimization
    reg_m_drop_addr(r_set, TEMP_ADDR_1);
    reg_m_drop_addr(r_set, TEMP_ADDR_2);
    reg_m_drop_addr(r_set, TEMP_ADDR_3);
    reg_m_drop_addr(r_set, TEMP_ADDR_4);
    reg_m_drop_addr(r_set, TEMP_ADDR_5);
}

void eval_EXPR(i_graph **i_current) {
    expression_t const * const expr = (*i_current)->payload;

    switch (expr->type) {
        case expr_VALUE:
            eval_expr_VALUE(expr);
            break;
        case expr_ADD:
            eval_expr_ARITHMETIC(expr,
                (arithmetic_func){ .func_num = &num_add, .func_reg = &arithm_ADD });
            break;
        case expr_SUB:
            eval_expr_ARITHMETIC(expr,
                (arithmetic_func){ .func_num = &num_sub, .func_reg = &arithm_SUB });
            break;
        case expr_MUL:
            eval_expr_ARITHMETIC(expr,
                (arithmetic_func){ .func_num = &num_mul, .func_reg = &arithm_MUL });
            break;
        case expr_DIV:
            eval_expr_ARITHMETIC(expr,
                (arithmetic_func){ .func_num = &num_div, .func_reg = &arithm_DIV });
            break;
        case expr_MOD:
            eval_expr_ARITHMETIC(expr,
                (arithmetic_func){ .func_num = &num_mod, .func_reg = &arithm_MOD });
            break;
        default:
            fprintf(stderr, "[EXPR]: Wrong type of expression!\n");
            exit(EXIT_FAILURE);
    }
}


static void expr_get_var_mask(expression_t const * const expr, bool check_second, bool mask_assign[2]) {
    symbol_table *s_table = get_symbol_table();

    const idx_t assign_idx = expr->var_1[0].sym_idx;
    symbol *assign_sym = symbol_table_find_by_idx(s_table, assign_idx);
    const bool assign_sym_1_is_array = assign_sym->flags & SYMBOL_IS_ARRAY;
    const bool assign_sym_2_num = expr->mask & ASSIGN_SYM2_NUM;
    const bool assign_sym_2_addr = expr->addr_mask & ASSIGN_SYM2_ADDR;


    const bool left_sym_1_addr = expr->addr_mask & LEFT_SYM1_ADDR;
    const bool left_sym_1_num = expr->mask & LEFT_SYM1_NUM;
    const bool left_sym_2_num = expr->mask & LEFT_SYM2_NUM;
    const bool left_sym_2_addr = expr->addr_mask & LEFT_SYM2_ADDR;
    if (left_sym_1_addr) {
        mask_assign[0] = false;
    } else if (!left_sym_1_num) {
        const idx_t left_idx = expr->var_1[1].sym_idx;
        if (left_idx == assign_idx) {
            if (assign_sym_1_is_array) {
                if (assign_sym_2_num) {
                    if (left_sym_2_num) {
                        mask_assign[0] = (expr->var_2[0].num == expr->var_2[1].num);
                    } else {
                        mask_assign[0] = false;
                    }
                } else if (assign_sym_2_addr) {
                    if (left_sym_2_addr) {
                        mask_assign[0] = (expr->var_2[0].addr == expr->var_2[1].addr);
                    } else {
                        mask_assign[0] = false;
                    }
                } else if (!left_sym_2_num && !left_sym_2_addr) {
                    mask_assign[0] = (expr->var_2[0].sym_idx == expr->var_2[1].sym_idx);
                } else {
                    mask_assign[0] = false;
                }
            } else {
                mask_assign[0] = true;
            }
        } else {
            mask_assign[0] = false;
        }
    } else {
        mask_assign[0] = false;
    }

    if (!check_second) {
        return ;
    }

    const bool right_sym_1_addr = expr->addr_mask & RIGHT_SYM1_ADDR;
    const bool right_sym_1_num = expr->mask & RIGHT_SYM1_NUM;
    const bool right_sym_2_num = expr->mask & RIGHT_SYM2_NUM;
    const bool right_sym_2_addr = expr->addr_mask & RIGHT_SYM2_ADDR;
    if (right_sym_1_addr) {
        mask_assign[1] = false;
    } else if (!right_sym_1_num) {
        const idx_t right_idx = expr->var_1[2].sym_idx;
        if (right_idx == assign_idx) {
            if (assign_sym_1_is_array) {
                if (assign_sym_2_num) {
                    if (right_sym_2_num) {
                        mask_assign[1] = (expr->var_2[0].num == expr->var_2[2].num);
                    } else {
                        mask_assign[1] = false;
                    }
                } else if (assign_sym_2_addr) {
                    if (right_sym_2_addr) {
                        mask_assign[1] = (expr->var_2[0].addr == expr->var_2[2].addr);
                    } else {
                        mask_assign[1] = false;
                    }
                } else if (!right_sym_2_num && !right_sym_2_addr) {
                    mask_assign[1] = (expr->var_2[0].sym_idx == expr->var_2[2].sym_idx);
                } else {
                    mask_assign[1] = false;
                }
            } else {
                mask_assign[1] = true;
            }
        } else {
            mask_assign[1] = false;
        }
    } else {
        mask_assign[1] = false;
    }
}
