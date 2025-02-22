#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "../std_oper/std_oper.h"
#include "../../register_machine/reg_m.h"

/**
 * return <= x + y
*/
val arithm_ADD(val x, val y, uint8_t *flags, bool mask_assign[2]);

/**
 * return <= x - y
*/
val arithm_SUB(val x, val y, uint8_t *flags, bool mask_assign[2]);

/** 
 * return <= x * y
*/
val arithm_MUL(val x, val y, uint8_t *flags, bool mask_assign[2]);

/** 
 * return <= x / y
*/
val arithm_DIV(val x, val y, uint8_t *flags, bool mask_assign[2]);

/** 
 * return <= x % y
*/
val arithm_MOD(val x, val y, uint8_t *flags, bool mask_assign[2]);

#endif
