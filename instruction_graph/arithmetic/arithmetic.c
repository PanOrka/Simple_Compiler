#include "arithmetic.h"

#include "../instructions/asm_fprintf.h"
#include "../../parser_func/getters.h"
#include "../generators/val_generator.h"

static void arithm_add_with_const(reg *x, mpz_t y) {
    reg_set *r_set = get_reg_set();

    oper_store_reg(x);
    if (mpz_cmp_ui(y, 5) <= 0) {
        const uint64_t y_const = mpz_get_ui(y);
        for (uint64_t i=0; i<y_const; ++i) {
            INC(x);
        }
    } else if (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated || !(reg_m_get(r_set, VAL_GEN_ADDR, false).r->flags & REG_MODIFIED)) {
        reg *val_gen = val_generate_from_mpz(y);
        ADD(x, val_gen);
    } else if (mpz_cmp_ui(y, 20) <= 0) {
        const uint64_t y_const = mpz_get_ui(y);
        for (uint64_t i=0; i<y_const; ++i) {
            INC(x);
        }
    } else {
        reg *val_gen = val_generate_from_mpz(y);
        ADD(x, val_gen);
    }

    mpz_clear(y);
}

reg * arithm_ADD(val *x, val *y) {
    if (x->is_reg && y->is_reg) {
        oper_store_reg(x->reg);
        if (x->reg == y->reg) {
            SHL(x->reg);
        } else {
            ADD(x->reg, y->reg);
        }

        return x->reg;
    } else if (x->is_reg) {
        arithm_add_with_const(x->reg, y->constant);

        return x->reg;
    } else {
        arithm_add_with_const(y->reg, x->constant);

        return y->reg;
    }
}

reg * arithm_MUL(val *x, val *y) {
    reg_set *r_set = get_reg_set();
    reg *acc = oper_get_reg_for_variable(TEMP_ADDR_3).r;
    acc->addr = TEMP_ADDR_3;
    RESET(acc);

    // additionally:
    // We can optimize by changing order of multiplication
    if (x == y) {
        y = oper_get_reg_for_variable(TEMP_ADDR_2).r;
        y->addr = TEMP_ADDR_2;
        oper_reg_swap(y, x);
    } else if (y->flags & REG_MODIFIED) {
        // JZERO_i_idx(y, ) <= totally not needed store if value under y is 0
        // JZERO_i_idx(x, ) <= totally not needed store if value under x is 0
        // ^ for reg_x it can even be outside this
        stack_ptr_generate(y->addr);
        STORE(y, &(r_set->stack_ptr));
        y->flags &= ~REG_MODIFIED;
    }

    JZERO_i_idx(x, 10);
    JZERO_i_idx(y, 9);
    JODD_i_idx(y, 4);
        SHL(x);
        SHR(y);
    JUMP_i_idx(-4);
        ADD(acc, x);
        SHL(x);
        SHR(y);
    JUMP_i_idx(-8);

    reg_m_drop_addr(r_set, x->addr);
    reg_m_drop_addr(r_set, y->addr);

    return acc;
}

reg * arithm_DIV(val *x, val *y) {
    reg_set *r_set = get_reg_set();

    // SAME AS IN MUL, totally not needed store if y is 0
    // Or if x is 0, for future optimizations
    reg *size = oper_get_reg_for_variable(TEMP_ADDR_3).r;
    size->addr = TEMP_ADDR_3;

    reg *rem = x;
    x = oper_get_reg_for_variable(TEMP_ADDR_4).r;
    x->addr = TEMP_ADDR_4;

    if (rem == y) {
        addr_t y_address = y->addr;
        rem->addr = TEMP_ADDR_1;
        y = oper_get_reg_for_variable(y_address).r;
        y->addr = y_address;
        oper_reg_swap(y, rem);
    } else {
        rem->addr = TEMP_ADDR_1; // to make sure it won't be stored + It's available for this register
    }

    reg *quotient = oper_get_reg_for_variable(TEMP_ADDR_5).r;
    quotient->addr = TEMP_ADDR_5;
    RESET(quotient);

    reg *temp = &(r_set->stack_ptr);
    stack_ptr_clear();

    /**
     * Integer division algorithm
     * pls kill me
    */
    JZERO_i_idx(y, 51); // If y is 0 then we end with 0
    DEC(y);
    JZERO_i_idx(y, 47); // If y is 1 then we end with x
    INC(y);

    // calculating size of reg_x value
    // and inversing x
    JZERO_i_idx(rem, 47); // end of proc x = 0 so END
    RESET(x);
    RESET(size);
    JODD_i_idx(rem, 4);
        SHR(rem);
        INC(size);
    JUMP_i_idx(-3); // If here then rem > 0, so waiting for odd
        INC(x);
        SHR(rem);
        INC(size);

    JZERO_i_idx(rem, 10); // end of proc, go further
        SHL(x);
    JODD_i_idx(rem, 4);
        SHR(rem);
        INC(size);
    JUMP_i_idx(-5); // check if rem is 0 and SHL x ^
        INC(x);
        SHR(rem);
        INC(size);
    JUMP_i_idx(-9);

    // now div algo rem is definetly 0 here
    JODD_i_idx(x, 4);
        DEC(size);
        SHR(x);
    JUMP_i_idx(-3); // x > 0 so to first ODD we have remainder = 0 divisor != 0
        INC(rem);
        DEC(size);
        SHR(x);
        // here we test 1 >= divisor so if divisor == 1 we should test it b4
        // return x if y == 1
    JZERO_i_idx(size, 20); // end of proc
        SHL(rem);
        SHL(quotient);
    JODD_i_idx(x, 11);
        DEC(size);
        SHR(x);

        RESET(temp); // R >= D
        ADD(temp, rem);
        INC(temp);
        SUB(temp, y);
        JZERO_i_idx(temp, -10); // jump to Jzero (end of proc)
            SUB(rem, y);
            INC(quotient);
    JUMP_i_idx(-13); // jump to Jzero (end of proc)
        INC(rem);
        DEC(size);
        SHR(x);
    JUMP_i_idx(-11); // jump to if R >= D

    INC(y); // reset y value when y = 1
    ADD(quotient, rem); // set quotient to x

    reg_m_promote(r_set, y->addr);
    return quotient;
}

reg * arithm_MOD(val *x, val *y) {
    reg_set *r_set = get_reg_set();

    // SAME AS IN MUL, totally not needed store if y is 0
    // Or if x is 0, for future optimizations
    reg *size = oper_get_reg_for_variable(TEMP_ADDR_3).r;
    size->addr = TEMP_ADDR_3;

    reg *rem = x;
    if (rem == y) {
        addr_t y_address = y->addr;
        rem->addr = TEMP_ADDR_1;
        y = oper_get_reg_for_variable(y_address).r;
        y->addr = y_address;
        oper_reg_swap(y, rem);
    } else {
        rem->addr = TEMP_ADDR_1; // to make sure it won't be stored + It's available for this register
    }
    x = oper_get_reg_for_variable(TEMP_ADDR_4).r;
    x->addr = TEMP_ADDR_4;

    reg *temp = &(r_set->stack_ptr);
    stack_ptr_clear();

    /**
     * Integer division algorithm
     * pls kill me
    */
    JZERO_i_idx(y, 48); // If y is 0 then we end with 0
    DEC(y);
    JZERO_i_idx(y, 45); // If y is 1 then we end with 0
    INC(y);

    // calculating size of reg_x value
    // and inversing x
    JZERO_i_idx(rem, 45); // end of proc x = 0 so END
    RESET(x);
    RESET(size);
    JODD_i_idx(rem, 4);
        SHR(rem);
        INC(size);
    JUMP_i_idx(-3); // If here then rem > 0, so waiting for odd
        INC(x);
        SHR(rem);
        INC(size);

    JZERO_i_idx(rem, 10); // end of proc, go further
        SHL(x);
    JODD_i_idx(rem, 4);
        SHR(rem);
        INC(size);
    JUMP_i_idx(-5); // check if rem is 0 and SHL x ^
        INC(x);
        SHR(rem);
        INC(size);
    JUMP_i_idx(-9);

    // now div algo rem is definetly 0 here
    JODD_i_idx(x, 4);
        DEC(size);
        SHR(x);
    JUMP_i_idx(-3); // x > 0 so to first ODD we have remainder = 0 divisor != 0
        INC(rem);
        DEC(size);
        SHR(x);
        // here we test 1 >= divisor so if divisor == 1 we should test it b4
        // return x if y == 1
    JZERO_i_idx(size, 18); // end of proc
        SHL(rem);
    JODD_i_idx(x, 10);
        DEC(size);
        SHR(x);

        RESET(temp); // R >= D
        ADD(temp, rem);
        INC(temp);
        SUB(temp, y);
        JZERO_i_idx(temp, -9); // jump to Jzero (end of proc)
            SUB(rem, y);
    JUMP_i_idx(-11); // jump to Jzero (end of proc)
        INC(rem);
        DEC(size);
        SHR(x);
    JUMP_i_idx(-10); // jump to if R >= D

    INC(y); // reset y value when y = 1
    RESET(rem);

    reg_m_promote(r_set, y->addr);
    return rem;
}
