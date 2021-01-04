#include "asm_fprintf.h"

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

void ADD(reg *x, reg *y) {
    fprintf(asm_out, "ADD %c %c\n", x->id, y->id);
}
void SUB(reg *x, reg *y) {
    fprintf(asm_out, "SUB %c %c\n", x->id, y->id);
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