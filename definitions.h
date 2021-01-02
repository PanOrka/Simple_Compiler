#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

typedef uint64_t addr_t;
typedef __int128_t num_t;

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

#endif
