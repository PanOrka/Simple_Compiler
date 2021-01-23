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

static void eval_expr_VALUE(expression_t const * const expr) {
    reg_set *r_set = get_reg_set();
    val assign_val = oper_get_assign_val_1(expr);
    oper_set_assign_val_0(expr, assign_val, ASSIGN_VAL_NO_FLAGS);
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
    val (*func_reg) (val x, val y);
} arithmetic_func;

static void eval_expr_ARITHMETIC(expression_t const * const expr, arithmetic_func func) {
    reg_set *r_set = get_reg_set();
    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);

    if (assign_val_1.is_reg) {
        reg_m_promote(r_set, assign_val_1.reg->addr);
    }

    // if (assign_val_1.is_reg && assign_val_2.is_reg) {
    //     reg_m_promote(r_set, assign_val_1.reg->addr);
    //     if (assign_val_1.reg->flags & REG_MODIFIED) { // First register is always stashed
    //         stack_ptr_generate(assign_val_1.reg->addr);
    //         STORE(assign_val_1.reg, &(r_set->stack_ptr));
    //         assign_val_1.reg->flags &= ~REG_MODIFIED;
    //     }

    //     reg *new_reg = func.func_reg(assign_val_1.reg, assign_val_2.reg);
    //     if (new_reg) {
    //         assign_val_1.reg = new_reg;
    //         reg_m_promote(r_set, assign_val_1.reg->addr);
    //     }

    //     oper_set_assign_val_0(expr, assign_val_1, ASSIGN_VAL_STASH);
    // } else if (assign_val_1.is_reg) {
    //     reg *val_reg = val_generate_from_mpz(assign_val_2.constant);
    //     mpz_clear(assign_val_2.constant);
    //     if (assign_val_1.reg->flags & REG_MODIFIED) { // First register is always stashed
    //         stack_ptr_generate(assign_val_1.reg->addr);
    //         STORE(assign_val_1.reg, &(r_set->stack_ptr));
    //         assign_val_1.reg->flags &= ~REG_MODIFIED;
    //     }

    //     reg *new_reg = func.func_reg(assign_val_1.reg, val_reg);
    //     if (new_reg) {
    //         assign_val_1.reg = new_reg;
    //         reg_m_promote(r_set, assign_val_1.reg->addr);
    //     }

    //     oper_set_assign_val_0(expr, assign_val_1, ASSIGN_VAL_STASH);
    // } else if (assign_val_2.is_reg) {
    //     reg *val_reg = val_generate_from_mpz(assign_val_1.constant);
    //     mpz_clear(assign_val_1.constant);
    //     val_reg->addr = TEMP_ADDR_1;

    //     reg *new_reg = func.func_reg(val_reg, assign_val_2.reg);
    //     if (new_reg) {
    //         assign_val_2.reg = new_reg;
    //         reg_m_promote(r_set, assign_val_2.reg->addr);
    //     } else {
    //         assign_val_2.reg = val_reg;
    //     }

    //     oper_set_assign_val_0(expr, assign_val_2, ASSIGN_VAL_STASH);
    // } else {
        
    // }

    if (!(assign_val_1.is_reg || assign_val_2.is_reg)) { // both constants
        func.func_num(assign_val_1.constant, assign_val_1.constant, assign_val_2.constant);
        mpz_clear(assign_val_2.constant);
        oper_set_assign_val_0(expr, assign_val_1, ASSIGN_VAL_NO_FLAGS);
        mpz_clear(assign_val_1.constant);
    } else {
        val new_val = func.func_reg(assign_val_1, assign_val_2);
        oper_set_assign_val_0(expr, new_val, ASSIGN_VAL_STASH);
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
