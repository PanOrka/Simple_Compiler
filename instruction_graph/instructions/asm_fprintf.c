#include "asm_fprintf.h"

#include "../../parser_func/getters.h"
#include "../std_oper/std_oper.h"
#include "../generators/stack_generator.h"

static FILE *asm_out = NULL;

static uint64_t i_num = 0;

uint64_t asm_get_i_num() {
    if (i_num == 0) {
        fprintf(stderr, "[ASM_FPRINTF]: i_num = 0\n");
        exit(EXIT_FAILURE);
    }

    return i_num - 1;
}

void asm_fprintf_set_file(FILE *file) {
    asm_out = file; 
}


void GET(reg *x) {
    fprintf(asm_out, "GET %c\n", x->id);
    ++i_num;
}

void PUT(reg *x) {
    fprintf(asm_out, "PUT %c\n", x->id);
    ++i_num;
}


void LOAD(reg *x, reg *y) {
    fprintf(asm_out, "LOAD %c %c\n", x->id, y->id);
    ++i_num;
}

void STORE(reg *x, reg *y) {
    fprintf(asm_out, "STORE %c %c\n", x->id, y->id);
    ++i_num;
}


reg * ADD(reg *x, reg *y) {
    fprintf(asm_out, "ADD %c %c\n", x->id, y->id);
    ++i_num;

    return NULL;
}

reg * SUB(reg *x, reg *y) {
    fprintf(asm_out, "SUB %c %c\n", x->id, y->id);
    ++i_num;

    return NULL;
}

reg * MUL(reg *x, reg *y) {
    reg_set *r_set = get_reg_set();
    reg *acc = oper_get_reg_for_variable(TEMP_ADDR_3).r;
    acc->addr = TEMP_ADDR_3;
    RESET(acc);

    // additionally:
    // We can optimize by changing order of multiplication
    if (y->flags & REG_MODIFIED) {
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

reg * DIV(reg *x, reg *y) {
    reg_set *r_set = get_reg_set();

    // SAME AS IN MUL, totally not needed store if y is 0
    // Or if x is 0, for future optimizations
    reg *size = oper_get_reg_for_variable(TEMP_ADDR_3).r;
    size->addr = TEMP_ADDR_3;

    reg *rem = x;
    x = oper_get_reg_for_variable(TEMP_ADDR_4).r;
    x->addr = TEMP_ADDR_4;

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

    reg_m_drop_addr(r_set, rem->addr);
    return quotient;
}

reg * MOD(reg *x, reg *y) {
    reg_set *r_set = get_reg_set();

    // SAME AS IN MUL, totally not needed store if y is 0
    // Or if x is 0, for future optimizations
    reg *size = oper_get_reg_for_variable(TEMP_ADDR_3).r;
    size->addr = TEMP_ADDR_3;

    reg *rem = x;
    rem->addr = TEMP_ADDR_1; // to make sure it won't be stored + It's available for this register
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
    JZERO_i_idx(rem, 46); // end of proc x = 0 so END
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

    return rem;
}

void RESET(reg *x) {
    fprintf(asm_out, "RESET %c\n", x->id);
    ++i_num;
}

void INC(reg *x) {
    fprintf(asm_out, "INC %c\n", x->id);
    ++i_num;
}

void DEC(reg *x) {
    fprintf(asm_out, "DEC %c\n", x->id);
    ++i_num;
}

void SHR(reg *x) {
    fprintf(asm_out, "SHR %c\n", x->id);
    ++i_num;
}

void SHL(reg *x) {
    fprintf(asm_out, "SHL %c\n", x->id);
    ++i_num;
}


void JUMP_i_idx(int64_t j) {
    fprintf(asm_out, "JUMP %ld\n", j);
    ++i_num;
}

void JZERO_i_idx(reg *x, int64_t j) {
    fprintf(asm_out, "JZERO %c %ld\n", x->id, j);
    ++i_num;
}

void JODD_i_idx(reg *x, int64_t j) {
    fprintf(asm_out, "JODD %c %ld\n", x->id, j);
    ++i_num;
}


void JUMP() {
    fprintf(asm_out, "JUMP X\n");
    ++i_num;
}

void JZERO(reg *x) {
    fprintf(asm_out, "JZERO %c X\n", x->id);
    ++i_num;
}

void JODD(reg *x) {
    fprintf(asm_out, "JODD %c X\n", x->id);
    ++i_num;
}


void HALT() {
    fprintf(asm_out, "HALT");
}
