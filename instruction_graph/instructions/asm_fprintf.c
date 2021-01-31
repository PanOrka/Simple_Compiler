#include "asm_fprintf.h"

#include "../../parser_func/getters.h"
#include "../std_oper/std_oper.h"
#include "../generators/stack_generator.h"

static FILE *asm_out = NULL;

static int64_t i_num = 0;

int64_t asm_get_i_num() {
    return i_num - 1;
}

void asm_fprintf_set_file(FILE *file) {
    asm_out = file; 
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
