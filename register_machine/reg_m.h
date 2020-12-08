#ifndef REG_M_H
#define REG_M_H

#if !defined REG_SIZE && !defined REG_ID_SET
#define REG_SIZE 6
#define REG_ID_SET {'a', 'b', 'c', 'd', 'e', 'f'}
#endif

#ifndef ADDR_UNDEF
#define ADDR_UNDEF 0xFFFFFFFFFFFFFFFF
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t addr;
    char id;
} reg;

typedef struct {
    reg r[REG_SIZE];
} reg_set;

typedef struct {
    reg *r;
    uint32_t idx;
    bool was_allocated;
} reg_allocator;

/**
 * 
 * Create register machine
 * 
 * RETURN: Copy of properly initialized register machine
 * 
*/
reg_set reg_m_create();

/**
 * 
 * Get register
 * If addr exists in reg_set then gives existing otherwise chooses Least Used Register
 * Returned register becomes 1st
 * 
 * RETURN: reg_allocator: ptr to register, old idx of register and was_allocated flag
 * 
*/
reg_allocator reg_m_get(uint64_t addr, reg_set *r_set);

/**
 * 
 * Returns Least Recently Used Register
 * Use it wisely for temporary objects that aren't addressed
 * Returned register becomes 1st
 * 
 * RETURN: reg_allocator: ptr to register, old idx of register and was_allocated flag
 * 
*/
reg_allocator reg_m_LRU(reg_set *r_set);

#endif
