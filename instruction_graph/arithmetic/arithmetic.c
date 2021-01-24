#include "arithmetic.h"

#include "../instructions/asm_fprintf.h"
#include "../../parser_func/getters.h"
#include "../generators/val_generator.h"
#include "../generators/stack_generator.h"

static void arithm_ADD_with_const(val *x, mpz_t y, uint8_t *flags) {
    reg_set *r_set = get_reg_set();
    const bool reg_no_store = !(x->reg->flags & REG_MODIFIED);

    if (mpz_cmp_ui(y, 0) == 0) {
        if (reg_no_store) {
            *flags = ASSIGN_VAL_STASH;
        } else {
            *flags = ASSIGN_VAL_NO_FLAGS;
        }
    } else if (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated || !(reg_m_get(r_set, VAL_GEN_ADDR, false).r->flags & REG_MODIFIED)) {
        reg *val_gen = val_generate_from_mpz(y);
        if (x->reg->addr == TEMP_ADDR_1 || x->reg->addr == TEMP_ADDR_2) {
            ADD(x->reg, val_gen);
        } else {
            val_gen->addr = TEMP_ADDR_3;
            ADD(val_gen, x->reg);
            x->reg = val_gen;
        }
        *flags = ASSIGN_VAL_STASH;
    } else if (reg_no_store && mpz_cmp_ui(y, 10) <= 0) {
        const uint64_t y_const = mpz_get_ui(y);
        for (uint64_t i=0; i<y_const; ++i) {
            INC(x->reg);
        }
        *flags = ASSIGN_VAL_STASH;
    } else {
        reg *val_gen = val_generate_from_mpz(y);
        if (x->reg->addr == TEMP_ADDR_1 || x->reg->addr == TEMP_ADDR_2) {
            ADD(x->reg, val_gen);
        } else {
            val_gen->addr = TEMP_ADDR_3;
            ADD(val_gen, x->reg);
            x->reg = val_gen;
        }
        *flags = ASSIGN_VAL_STASH;
    }

    mpz_clear(y);
}

val arithm_ADD(val x, val y, uint8_t *flags) {
    if (x.is_reg && y.is_reg) {
        oper_store_reg(x.reg);
        if (x.reg == y.reg) {
            SHL(x.reg);
        } else {
            ADD(x.reg, y.reg);
        }

        *flags = ASSIGN_VAL_STASH;
        return x;
    } else if (x.is_reg) {
        arithm_ADD_with_const(&x, y.constant, flags);

        return x;
    } else {
        arithm_ADD_with_const(&y, x.constant, flags);

        return y;
    }
}

static void arithm_SUB_with_const(val *x, mpz_t y, uint8_t *flags) {
    reg_set *r_set = get_reg_set();
    const bool reg_no_store = !(x->reg->flags & REG_MODIFIED);

    if (mpz_cmp_ui(y, 0) == 0) {
        if (reg_no_store) {
            *flags = ASSIGN_VAL_STASH;
        } else {
            *flags = ASSIGN_VAL_NO_FLAGS;
        }
    } else if (reg_no_store &&
                    (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated || !(reg_m_get(r_set, VAL_GEN_ADDR, false).r->flags & REG_MODIFIED))) {

        reg *val_gen = val_generate_from_mpz(y);
        SUB(x->reg, val_gen);
        *flags = ASSIGN_VAL_STASH;
    } else if (mpz_cmp_ui(y, 10) <= 0) {
        oper_store_reg(x->reg);
        const uint64_t y_const = mpz_get_ui(y);
        for (uint64_t i=0; i<y_const; ++i) {
            DEC(x->reg);
        }
        *flags = ASSIGN_VAL_STASH;
    } else {
        reg *val_gen = val_generate_from_mpz(y);
        oper_store_reg(x->reg);
        SUB(x->reg, val_gen);
        *flags = ASSIGN_VAL_STASH;
    }

    mpz_clear(y);
}

val arithm_SUB(val x, val y, uint8_t *flags) {
    if (x.is_reg && y.is_reg) {
        if (x.reg == y.reg) {
            val new_val;
            new_val.is_reg = false;
            mpz_init_set_si(new_val.constant, 0);

            *flags = ASSIGN_VAL_NO_FLAGS;
            return new_val;
        }

        oper_store_reg(x.reg);
        SUB(x.reg, y.reg);

        *flags = ASSIGN_VAL_STASH;
        return x;
    } else if (x.is_reg) {
        arithm_SUB_with_const(&x, y.constant, flags);

        return x;
    } else {
        if (mpz_cmp_ui(x.constant, 0) == 0) {
            mpz_clear(x.constant);

            val new_val;
            new_val.is_reg = false;
            mpz_init_set_si(new_val.constant, 0);

            *flags = ASSIGN_VAL_NO_FLAGS;
            return new_val;
        } else {
            reg *val_gen = val_generate_from_mpz(x.constant);
            mpz_clear(x.constant);
            val_gen->addr = TEMP_ADDR_3;

            SUB(val_gen, y.reg);
            *flags = ASSIGN_VAL_STASH;
            return (val){ .is_reg = true, .reg = val_gen };
        }
    }
}

static void arithm_MUL_with_const(val *x, mpz_t y, uint8_t *flags) {
    const bool reg_no_store = !(x->reg->flags & REG_MODIFIED);

    if (mpz_cmp_ui(y, 0) == 0) {
        val new_val;
        new_val.is_reg = false;
        mpz_init_set_si(new_val.constant, 0);

        *flags = ASSIGN_VAL_NO_FLAGS;
        *x = new_val;
    } else if (mpz_cmp_ui(y, 1) == 0) {
        if (reg_no_store) {
            *flags = ASSIGN_VAL_STASH;
        } else {
            *flags = ASSIGN_VAL_NO_FLAGS;
        }
    } else {
        mp_bitcnt_t bits = mpz_popcount(y);
        if (bits == 1) {
            oper_store_reg(x->reg);
            while (mpz_cmp_ui(y, 1) != 0) {
                mpz_div_ui(y, y, 2);
                SHL(x->reg);
            }
            *flags = ASSIGN_VAL_STASH;
        } else {
            reg *val_gen = val_generate_from_mpz(y);
            val_gen->addr = TEMP_ADDR_4;
            *x = arithm_MUL(*x, (val) { .is_reg = true, .reg = val_gen }, flags);
        }
    }

    mpz_clear(y);
}

val arithm_MUL(val x, val y, uint8_t *flags) {
    if (x.is_reg && y.is_reg) {
        reg_set *r_set = get_reg_set();

        oper_store_reg(x.reg);

        reg *acc = oper_get_reg_for_variable(TEMP_ADDR_3).r;
        acc->addr = TEMP_ADDR_3;

        if (x.reg == y.reg) {
            y.reg = oper_get_reg_for_variable(TEMP_ADDR_2).r;
            y.reg->addr = TEMP_ADDR_2;
            oper_reg_swap(y.reg, x.reg);

            RESET(acc);
            JZERO_i_idx(x.reg, 10);
            JZERO_i_idx(y.reg, 9);
            JODD_i_idx(y.reg, 4);
                SHL(x.reg);
                SHR(y.reg);
            JUMP_i_idx(-4);
                ADD(acc, x.reg);
                SHL(x.reg);
                SHR(y.reg);
            JUMP_i_idx(-8);
        } else {
            oper_store_reg(y.reg);

            JZERO_i_idx(x.reg, 39); // JUMP to RESET(acc) <- we return 0
            JZERO_i_idx(y.reg, 38); // JUMP to RESET(acc) <- we return 0
                oper_reg_swap(acc, x.reg); // 2 instructions
                SUB(acc, y.reg);
                // HERE x > y
                JZERO_i_idx(acc, 18); // JUMP TO IS_ODD
                    RESET(acc);
                    // unwind once
                    JODD_i_idx(y.reg, 4);
                        SHL(x.reg);
                        SHR(y.reg);
                    JUMP_i_idx(4);
                        ADD(acc, x.reg);
                        SHL(x.reg);
                        SHR(y.reg);
                    //////////////
                    JZERO_i_idx(y.reg, 26); // JUMP outside
                    JODD_i_idx(y.reg, 4);
                        SHL(x.reg);
                        SHR(y.reg);
                    JUMP_i_idx(-4);
                        ADD(acc, x.reg);
                        SHL(x.reg);
                        SHR(y.reg);
                    JUMP_i_idx(-8);
                // HERE x <= y
                    // unwind once
                    JODD_i_idx(x.reg, 4); // IS_ODD
                        SHL(y.reg);
                        SHR(x.reg);
                    JUMP_i_idx(4);
                        ADD(acc, y.reg);
                        SHL(y.reg);
                        SHR(x.reg);
                    //////////////
                    JZERO_i_idx(x.reg, 10); // JUMP outside
                    JODD_i_idx(x.reg, 4);
                        SHL(y.reg);
                        SHR(x.reg);
                    JUMP_i_idx(-4);
                        ADD(acc, y.reg);
                        SHL(y.reg);
                        SHR(x.reg);
                    JUMP_i_idx(-8);
            RESET(acc);
        }

        reg_m_drop_addr(r_set, x.reg->addr);
        reg_m_drop_addr(r_set, y.reg->addr);

        *flags = ASSIGN_VAL_STASH;
        return (val){ .is_reg = true, .reg = acc };
    } else if (x.is_reg) {
        arithm_MUL_with_const(&x, y.constant, flags);
        return x;
    } else {
        arithm_MUL_with_const(&y, x.constant, flags);
        return y;
    }
}

static void arithm_DIV_with_const(val *x, mpz_t y, uint8_t *flags) {
    const bool reg_no_store = !(x->reg->flags & REG_MODIFIED);

    if (mpz_cmp_ui(y, 0) == 0) {
        val new_val;
        new_val.is_reg = false;
        mpz_init_set_si(new_val.constant, 0);

        *flags = ASSIGN_VAL_NO_FLAGS;
        *x = new_val;
    } else if (mpz_cmp_ui(y, 1) == 0) {
        if (reg_no_store) {
            *flags = ASSIGN_VAL_STASH;
        } else {
            *flags = ASSIGN_VAL_NO_FLAGS;
        }
    } else {
        mp_bitcnt_t bits = mpz_popcount(y);
        if (bits == 1) {
            oper_store_reg(x->reg);
            while (mpz_cmp_ui(y, 1) != 0) {
                mpz_div_ui(y, y, 2);
                SHR(x->reg);
            }
            *flags = ASSIGN_VAL_STASH;
        } else {
            reg *val_gen = val_generate_from_mpz(y);
            *x = arithm_DIV(*x, (val) { .is_reg = true, .reg = val_gen }, flags);
        }
    }

    mpz_clear(y);
}

val arithm_DIV(val x, val y, uint8_t *flags) {
    if (x.is_reg && y.is_reg) {
        if (x.reg == y.reg) {
            val new_val;
            new_val.is_reg = false;
            mpz_init_set_si(new_val.constant, 1);

            *flags = ASSIGN_VAL_NO_FLAGS;
            return new_val;
        }
        oper_store_reg(x.reg);

        reg_set *r_set = get_reg_set();

        reg *size = oper_get_reg_for_variable(TEMP_ADDR_3).r;
        size->addr = TEMP_ADDR_3;

        reg *rem = x.reg;
        x.reg = oper_get_reg_for_variable(TEMP_ADDR_4).r;
        x.reg->addr = TEMP_ADDR_4;

        rem->addr = TEMP_ADDR_1; // to make sure it won't be stored + It's available for this register

        reg *quotient = oper_get_reg_for_variable(TEMP_ADDR_5).r;
        quotient->addr = TEMP_ADDR_5;
        RESET(quotient);

        reg *temp = &(r_set->stack_ptr);
        stack_ptr_clear();

        /**
         * Integer division algorithm
         * pls kill me
        */
        JZERO_i_idx(y.reg, 51); // If y is 0 then we end with 0
        DEC(y.reg);
        JZERO_i_idx(y.reg, 47); // If y is 1 then we end with x
        INC(y.reg);

        // calculating size of reg_x value
        // and inversing x
        JZERO_i_idx(rem, 47); // end of proc x = 0 so END
        RESET(x.reg);
        RESET(size);
        JODD_i_idx(rem, 4);
            SHR(rem);
            INC(size);
        JUMP_i_idx(-3); // If here then rem > 0, so waiting for odd
            INC(x.reg);
            SHR(rem);
            INC(size);

        JZERO_i_idx(rem, 10); // end of proc, go further
            SHL(x.reg);
        JODD_i_idx(rem, 4);
            SHR(rem);
            INC(size);
        JUMP_i_idx(-5); // check if rem is 0 and SHL x ^
            INC(x.reg);
            SHR(rem);
            INC(size);
        JUMP_i_idx(-9);

        // now div algo rem is definetly 0 here
        JODD_i_idx(x.reg, 4);
            DEC(size);
            SHR(x.reg);
        JUMP_i_idx(-3); // x > 0 so to first ODD we have remainder = 0 divisor != 0
            INC(rem);
            DEC(size);
            SHR(x.reg);
            // here we test 1 >= divisor so if divisor == 1 we should test it b4
            // return x if y == 1
        JZERO_i_idx(size, 20); // end of proc
            SHL(rem);
            SHL(quotient);
        JODD_i_idx(x.reg, 11);
            DEC(size);
            SHR(x.reg);

            RESET(temp); // R >= D
            ADD(temp, rem);
            INC(temp);
            SUB(temp, y.reg);
            JZERO_i_idx(temp, -10); // jump to Jzero (end of proc)
                SUB(rem, y.reg);
                INC(quotient);
        JUMP_i_idx(-13); // jump to Jzero (end of proc)
            INC(rem);
            DEC(size);
            SHR(x.reg);
        JUMP_i_idx(-11); // jump to if R >= D

        INC(y.reg); // reset y value when y = 1
        ADD(quotient, rem); // set quotient to x

        reg_m_promote(r_set, y.reg->addr);
        *flags = ASSIGN_VAL_STASH;
        return (val) { .is_reg = true, .reg = quotient };
    } else if (x.is_reg) {
        arithm_DIV_with_const(&x, y.constant, flags);
        return x;
    } else {
        if (mpz_cmp_ui(x.constant, 0) == 0) {
            mpz_clear(x.constant);

            val new_val;
            new_val.is_reg = false;
            mpz_init_set_si(new_val.constant, 0);

            *flags = ASSIGN_VAL_NO_FLAGS;
            return new_val;
        } else {
            reg *val_gen = val_generate_from_mpz(x.constant);
            mpz_clear(x.constant);
            val_gen->addr = TEMP_ADDR_2;

            return arithm_DIV((val){ .is_reg = true, .reg = val_gen }, y, flags);
        }
    }
}

static void arithm_MOD_with_const(val *x, mpz_t y, uint8_t *flags) {
    if (mpz_cmp_ui(y, 0) == 0 || mpz_cmp_ui(y, 1) == 0) {
        val new_val;
        new_val.is_reg = false;
        mpz_init_set_si(new_val.constant, 0);

        *flags = ASSIGN_VAL_NO_FLAGS;
        *x = new_val;
    } else if (mpz_cmp_ui(y, 2) == 0) {
        reg *new_reg = oper_get_reg_for_variable(TEMP_ADDR_3).r;
        new_reg->addr = TEMP_ADDR_3;
        *flags = ASSIGN_VAL_STASH;

        JODD_i_idx(x->reg, 3);
            RESET(new_reg);
        JUMP_i_idx(3);
            RESET(new_reg); // ODD
            INC(new_reg);
        // ENDIF
        x->reg = new_reg;
    } else {
        reg *val_gen = val_generate_from_mpz(y);
        *x = arithm_MOD(*x, (val) { .is_reg = true, .reg = val_gen }, flags);
    }

    mpz_clear(y);
}

val arithm_MOD(val x, val y, uint8_t *flags) {
    if (x.is_reg && y.is_reg) {
        if (x.reg == y.reg) {
            val new_val;
            new_val.is_reg = false;
            mpz_init_set_si(new_val.constant, 0);

            *flags = ASSIGN_VAL_NO_FLAGS;
            return new_val;
        }
        oper_store_reg(x.reg);

        reg_set *r_set = get_reg_set();

        reg *size = oper_get_reg_for_variable(TEMP_ADDR_3).r;
        size->addr = TEMP_ADDR_3;

        reg *rem = x.reg;
        x.reg = oper_get_reg_for_variable(TEMP_ADDR_4).r;
        x.reg->addr = TEMP_ADDR_4;

        rem->addr = TEMP_ADDR_1; // to make sure it won't be stored + It's available for this register

        reg *temp = &(r_set->stack_ptr);
        stack_ptr_clear();

        /**
         * Integer division algorithm
         * pls kill me
        */
        JZERO_i_idx(y.reg, 48); // If y is 0 then we end with 0
        DEC(y.reg);
        JZERO_i_idx(y.reg, 45); // If y is 1 then we end with 0
        INC(y.reg);

        // calculating size of reg_x value
        // and inversing x
        JZERO_i_idx(rem, 45); // end of proc x = 0 so END
        RESET(x.reg);
        RESET(size);
        JODD_i_idx(rem, 4);
            SHR(rem);
            INC(size);
        JUMP_i_idx(-3); // If here then rem > 0, so waiting for odd
            INC(x.reg);
            SHR(rem);
            INC(size);

        JZERO_i_idx(rem, 10); // end of proc, go further
            SHL(x.reg);
        JODD_i_idx(rem, 4);
            SHR(rem);
            INC(size);
        JUMP_i_idx(-5); // check if rem is 0 and SHL x ^
            INC(x.reg);
            SHR(rem);
            INC(size);
        JUMP_i_idx(-9);

        // now div algo rem is definetly 0 here
        JODD_i_idx(x.reg, 4);
            DEC(size);
            SHR(x.reg);
        JUMP_i_idx(-3); // x > 0 so to first ODD we have remainder = 0 divisor != 0
            INC(rem);
            DEC(size);
            SHR(x.reg);
            // here we test 1 >= divisor so if divisor == 1 we should test it b4
            // return x if y == 1
        JZERO_i_idx(size, 18); // end of proc
            SHL(rem);
        JODD_i_idx(x.reg, 10);
            DEC(size);
            SHR(x.reg);

            RESET(temp); // R >= D
            ADD(temp, rem);
            INC(temp);
            SUB(temp, y.reg);
            JZERO_i_idx(temp, -9); // jump to Jzero (end of proc)
                SUB(rem, y.reg);
        JUMP_i_idx(-11); // jump to Jzero (end of proc)
            INC(rem);
            DEC(size);
            SHR(x.reg);
        JUMP_i_idx(-10); // jump to if R >= D

        INC(y.reg); // reset y value when y = 1
        RESET(rem);

        reg_m_promote(r_set, y.reg->addr);
        *flags = ASSIGN_VAL_STASH;
        return (val){ .is_reg = true, .reg = rem };
    } else if (x.is_reg) {
        arithm_MOD_with_const(&x, y.constant, flags);
        return x;
    } else {
        if (mpz_cmp_ui(x.constant, 0) == 0) {
            mpz_clear(x.constant);

            val new_val;
            new_val.is_reg = false;
            mpz_init_set_si(new_val.constant, 0);

            *flags = ASSIGN_VAL_NO_FLAGS;
            return new_val;
        } else if (mpz_cmp_ui(x.constant, 1) == 0) {
            mpz_clear(x.constant);

            reg *new_reg = oper_get_reg_for_variable(TEMP_ADDR_3).r;
            new_reg->addr = TEMP_ADDR_3;
            *flags = ASSIGN_VAL_STASH;

            JZERO_i_idx(y.reg, 10);
                DEC(y.reg);
                JZERO_i_idx(y.reg, 5);
                    INC(y.reg);
                    RESET(new_reg);
                    INC(new_reg);
                JUMP_i_idx(5);
                    INC(y.reg);
                    RESET(new_reg);
            JUMP_i_idx(2);
                RESET(new_reg);
            // ENDIF
            return (val){ .is_reg = true, .reg = new_reg };
        } else {
            reg *val_gen = val_generate_from_mpz(x.constant);
            mpz_clear(x.constant);
            val_gen->addr = TEMP_ADDR_2;

            return arithm_MOD((val){ .is_reg = true, .reg = val_gen }, y, flags);
        }
    }
}
