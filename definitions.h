#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

typedef uint64_t addr_t;

/**
 * 
 * This is for undefined values (those without address)
 * 
*/
#ifndef ADDR_UNDEF
#define ADDR_UNDEF 0xFFFFFFFFFFFFFFFF
#endif

/**
 * 
 * Address specified for stack pointer register
 * 
*/
#ifndef STACK_PTR
#define STACK_PTR 0xFFFFFFFFFFFFFFFE
#endif

/**
 * 
 * MSB
 * 
*/
#ifndef ADDR_T_MSB
#define ADDR_T_MSB 0x8000000000000000
#endif

/**
 * 
 * VAL_GEN_ADDR - address specified for value generator
 * 
*/
#ifndef VAL_GEN_ADDR
#define VAL_GEN_ADDR 0xFFFFFFFFFFFFFFFD
#endif

/**
 * 
 * TEMP_ADDR_1 - temporary address for later drop, !DON'T LEAVE THAT IN REGISTERS!
 * OR don't give flag REG_MODIFIED
 * 
*/
#ifndef TEMP_ADDR_1
#define TEMP_ADDR_1 0xFFFFFFFFFFFFFFFC
#endif

/**
 * 
 * TEMP_ADDR_2
 * 
*/
#ifndef TEMP_ADDR_2
#define TEMP_ADDR_2 0xFFFFFFFFFFFFFFFB
#endif

/**
 * 
 * TEMP_ADDR_3
 * 
*/
#ifndef TEMP_ADDR_3
#define TEMP_ADDR_3 0xFFFFFFFFFFFFFFFA
#endif

/**
 * 
 * TEMP_ADDR_4
 * 
*/
#ifndef TEMP_ADDR_4
#define TEMP_ADDR_4 0xFFFFFFFFFFFFFFF9
#endif

/**
 * 
 * TEMP_ADDR_5
 * 
*/
#ifndef TEMP_ADDR_5
#define TEMP_ADDR_5 0xFFFFFFFFFFFFFFF8
#endif

#endif
