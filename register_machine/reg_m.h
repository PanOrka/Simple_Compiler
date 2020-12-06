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
    char id;
    bool is_allocated;
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
 * Put addr on register machine
 * If exists then gives existing otherwise chooses Least Used Register
 * 
 * RETURN: reg_allocator: id of register && is_allocated flag
 * 
*/
reg_allocator reg_m_put_addr(uint64_t addr);

/**
 * 
 * Change stored addr to r.addr of register specified by r.id
 * 
 * WARNING: Used ONLY in instructions of type OPER reg_x reg_y = reg_x <- reg_x OPER reg_y, when destination is change ex: c := a + b
*/
void reg_m_change_addr(reg r);

#endif
