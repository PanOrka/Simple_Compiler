#ifndef ASM_FPRINTF_H
#define ASM_FPRINTF_H

#include "../../register_machine/reg_m.h"

#include <stdio.h>

/**
 * 
 * Set file to print ASM
 * 
*/
void asm_fprintf_set_file(FILE *file);

int64_t asm_get_i_num();

void GET(reg *x);
void PUT(reg *x);

void LOAD(reg *x, reg *y);
void STORE(reg *x, reg *y);

reg * ADD(reg *x, reg *y);
reg * SUB(reg *x, reg *y);

void RESET(reg *x);
void INC(reg *x);
void DEC(reg *x);
void SHR(reg *x);
void SHL(reg *x);

void JUMP_i_idx(int64_t j);
void JZERO_i_idx(reg *x, int64_t j);
void JODD_i_idx(reg *x, int64_t j);

void JUMP();
void JZERO(reg *x);
void JODD(reg *x);

void HALT();

#endif
