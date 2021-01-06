#include "i_graph.h"
#include "expr_checker.h"
#include "../parser_func/getters.h"
#include "generators/stack_generator.h"
#include "generators/val_generator.h"
#include "std_oper/std_oper.h"
#include "instructions/asm_fprintf.h"

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
    reg *assign_val = oper_get_assign_val_1(expr);
    uint8_t const assign_val_flags = (expr->mask & LEFT_SYM1_NUM) ? ASSIGN_VAL_IS_NUM : ASSIGN_VAL_NO_FLAGS;
    oper_set_assign_val_0(expr, assign_val, assign_val_flags);

    reg_m_drop_addr(r_set, TEMP_ADDR_1);
}

static void num_add(mpz_t val, int64_t x, int64_t y) {
    mpz_set_si(val, x);
    mpz_add_ui(val, val, (uint64_t)y);
}

static void num_sub(mpz_t val, int64_t x, int64_t y) {
    if (x - y > 0) {
        mpz_set_si(val, x);
        mpz_sub_ui(val, val, (uint64_t)y);
    }
}

static void num_mul(mpz_t val, int64_t x, int64_t y) {
    mpz_set_si(val, x);
    mpz_mul_si(val, val, y);
}

static void num_div(mpz_t val, int64_t x, int64_t y) {
    if (y != 0) {
        mpz_set_si(val, x);
        mpz_tdiv_q_ui(val, val, (uint64_t)y);
    }
}

static void num_mod(mpz_t val, int64_t x, int64_t y) {
    if (y != 0) {
        mpz_set_si(val, x);
        mpz_tdiv_r_ui(val, val, (uint64_t)y);
    }
}

typedef struct {
    void (*func_num) (mpz_t val, int64_t x, int64_t y);
    reg * (*func_reg) (reg *x, reg *y);
} arithmetic_func;

static void eval_expr_ARITHMETIC(expression_t const * const expr, arithmetic_func func) {
    reg_set *r_set = get_reg_set();
    
    if ((expr->mask & LEFT_SYM1_NUM) && (expr->mask & RIGHT_SYM1_NUM)) {
        mpz_t val;
        mpz_init(val);
        func.func_num(val, expr->var_1[1].num, expr->var_1[2].num);

        reg *assign_val = val_generate_from_mpz(val);
        mpz_clear(val);

        oper_set_assign_val_0(expr, assign_val, ASSIGN_VAL_IS_NUM);
    } else {
        // FOR NOW it's glued up with TODO in std_oper get_assign functions
        // Could be optimized:
        // - save or stash VAL_GEN_ADDR register depending on situation with other variables
        // - definetly let TEMP_ADDR_1 or TEMP_ADDR_2 to be sum
        // - reuse code - use it for abelowe operacje
        // - consider var_1 := var_1 (oper) var_2 for optimization
        reg *assign_val_1 = oper_get_assign_val_1(expr);
        if (assign_val_1->addr == VAL_GEN_ADDR) {
            assign_val_1->addr = TEMP_ADDR_1;
        } else if (assign_val_1->flags & REG_MODIFIED) { // First register is always stashed
            stack_ptr_generate(assign_val_1->addr);
            STORE(assign_val_1, &(r_set->stack_ptr));
            assign_val_1->flags &= ~REG_MODIFIED;
        }

        reg *assign_val_2 = oper_get_assign_val_2(expr);
        reg_m_promote(r_set, assign_val_1->addr);
        if (assign_val_1->addr == VAL_GEN_ADDR) {
            assign_val_1->addr = TEMP_ADDR_2;
        }

        reg * new_reg = func.func_reg(assign_val_1, assign_val_2);
        if (new_reg) {
            assign_val_1 = new_reg;
            reg_m_promote(r_set, assign_val_1->addr);
        }

        oper_set_assign_val_0(expr, assign_val_1, ASSIGN_VAL_STASH);

        reg_m_drop_addr(r_set, TEMP_ADDR_1);
        reg_m_drop_addr(r_set, TEMP_ADDR_2);
        reg_m_drop_addr(r_set, TEMP_ADDR_3);
        reg_m_drop_addr(r_set, TEMP_ADDR_4);
        reg_m_drop_addr(r_set, TEMP_ADDR_5);
    }
}

void eval_EXPR(i_graph **i_current) {
    expression_t const * const expr_curr = (*i_current)->payload;

    switch (expr_curr->type) {
        case expr_VALUE:
            eval_expr_VALUE(expr_curr);
            break;
        case expr_ADD:
            eval_expr_ARITHMETIC(expr_curr,
                (arithmetic_func){ .func_num = &num_add, .func_reg = &ADD });
            break;
        case expr_SUB:
            eval_expr_ARITHMETIC(expr_curr,
                (arithmetic_func){ .func_num = &num_sub, .func_reg = &SUB });
            break;
        case expr_MUL:
            eval_expr_ARITHMETIC(expr_curr,
                (arithmetic_func){ .func_num = &num_mul, .func_reg = &MUL });
            break;
        case expr_DIV:
            eval_expr_ARITHMETIC(expr_curr,
                (arithmetic_func){ .func_num = &num_div, .func_reg = &DIV });
            break;
        case expr_MOD:
            eval_expr_ARITHMETIC(expr_curr,
                (arithmetic_func){ .func_num = &num_mod, .func_reg = &MOD });
            break;
        default:
            fprintf(stderr, "[EXPR]: Wrong type of expression!\n");
            exit(EXIT_FAILURE);
    }
}
