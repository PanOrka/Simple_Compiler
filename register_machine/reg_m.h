#ifndef REG_M_H
#define REG_M_H

#if !defined REG_SIZE && !defined REG_ID_SET
#define REG_SIZE 5
#define REG_ID_SET {'a', 'b', 'c', 'd', 'e', 'f'}
#endif

#include <stdbool.h>
#include <gmp.h>

#include "../definitions.h"

#ifndef REG_NO_FLAGS
#define REG_NO_FLAGS 0b00000000
#endif

#ifndef REG_MODIFIED
#define REG_MODIFIED 0b10000000
#endif

typedef struct {
    addr_t addr;
    char id;
    uint8_t flags;
} reg;

typedef struct {
    reg *r[REG_SIZE];
    reg stack_ptr;
} reg_set;

typedef struct {
    reg *r;
    uint32_t idx;
    bool was_allocated;
} reg_allocator;

/**
 * stack_ptr is cleared before snapshot! 
*/
typedef struct {
    reg r[REG_SIZE];
    mpz_t stack_ptr_value;
    mpz_t val_gen_value;
} reg_snapshot;

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
 * RETURN: reg_allocator: ptr to register set, old idx of register and was_allocated flag
 * 
*/
reg_allocator reg_m_get(reg_set *r_set, addr_t addr, bool do_sort);

/**
 * 
 * Returns Least Recently Used Register
 * Use it wisely for temporary objects that aren't addressed, best to use for single fast operations
 * Returned register becomes 1st
 * 
 * RETURN: reg_allocator: ptr to register set, old idx of register and was_allocated flag
 * 
*/
reg_allocator reg_m_LRU(reg_set *r_set, bool do_sort);

/**
 * 
 * Drops register with given address
 * Used mainly to drop iterators and FORLOOP variables
 * 
*/
void reg_m_drop_addr(reg_set *r_set, addr_t addr);

/**
 * 
 * Promotes register with given address
 * Used to promote register after assignment
 * 
*/
void reg_m_promote(reg_set *r_set, addr_t addr);

/**
 * 
 * Return snapshot of reg_m
 * 
 * WARNING: USES MPZ_INIT to initialize bignums
*/
reg_snapshot reg_m_snapshot(reg_set *r_set);

/**
 * 
 * Apply snapshot
 * 
*/
void reg_m_apply_snapshot(reg_set *r_set, reg_snapshot r_snap);

#endif
