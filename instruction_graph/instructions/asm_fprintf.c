#include "asm_fprintf.h"

#include "../../parser_func/getters.h"
#include "../std_oper/std_oper.h"

static FILE *asm_out = NULL;

void asm_fprintf_set_file(FILE *file) {
    asm_out = file; 
}


void GET(reg *x) {
    fprintf(asm_out, "GET %c\n", x->id);
}

void PUT(reg *x) {
    fprintf(asm_out, "PUT %c\n", x->id);
}


void LOAD(reg *x, reg *y) {
    fprintf(asm_out, "LOAD %c %c\n", x->id, y->id);
}

void STORE(reg *x, reg *y) {
    fprintf(asm_out, "STORE %c %c\n", x->id, y->id);
}


reg * ADD(reg *x, reg *y) {
    fprintf(asm_out, "ADD %c %c\n", x->id, y->id);

    return NULL;
}

reg * SUB(reg *x, reg *y) {
    fprintf(asm_out, "SUB %c %c\n", x->id, y->id);

    return NULL;
}

reg * MUL(reg *x, reg *y) {
    reg_set *r_set = get_reg_set();
    reg *acc = oper_get_reg_for_variable(TEMP_ADDR_3).r;
    acc->addr = TEMP_ADDR_3;
    RESET(acc);

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


void RESET(reg *x) {
    fprintf(asm_out, "RESET %c\n", x->id);
}

void INC(reg *x) {
    fprintf(asm_out, "INC %c\n", x->id);
}

void DEC(reg *x) {
    fprintf(asm_out, "DEC %c\n", x->id);
}

void SHR(reg *x) {
    fprintf(asm_out, "SHR %c\n", x->id);
}

void SHL(reg *x) {
    fprintf(asm_out, "SHL %c\n", x->id);
}


void JUMP_i_idx(int64_t j) {
    fprintf(asm_out, "JUMP %ld\n", j);
}

void JZERO_i_idx(reg *x, int64_t j) {
    fprintf(asm_out, "JZERO %c %ld\n", x->id, j);
}

void JODD_i_idx(reg *x, int64_t j) {
    fprintf(asm_out, "JODD %c %ld\n", x->id, j);
}


void JUMP() {

}

void JZERO(reg *x) {

}

void JODD(reg *x) {

}


void HALT() {
    fprintf(asm_out, "HALT");
}
