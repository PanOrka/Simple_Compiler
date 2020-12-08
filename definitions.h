#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

typedef uint64_t addr_t;
typedef uint32_t instr_lvl_t;

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

#endif
