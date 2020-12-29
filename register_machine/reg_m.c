#include "reg_m.h"

#include <stdio.h>
#include <stdlib.h>

reg_set reg_m_create() {
    char reg_id_arr[REG_SIZE + 1] = REG_ID_SET;
    reg_set regs;

    for (int32_t i=0; i<REG_SIZE; ++i) {
        regs.r[i] = malloc(sizeof(reg));
        if (!(regs.r[i])) {
            fprintf(stderr, "[REG_M]: Couldn't allocate memory for register!\n");
            exit(EXIT_FAILURE);
        }
        regs.r[i]->addr = ADDR_UNDEF;
        regs.r[i]->id = reg_id_arr[i];
        regs.r[i]->flags = REG_NO_FLAGS;
    }

    regs.stack_ptr.addr = STACK_PTR;
    regs.stack_ptr.id = reg_id_arr[REG_SIZE];
    regs.stack_ptr.flags = REG_NO_FLAGS;

    return regs;
}

#ifndef REG_M_SORT_UP
#define REG_M_SORT_UP 1
#endif

#ifndef REG_M_SORT_DOWN
#define REG_M_SORT_DOWN -1
#endif

void reg_m_sort(reg_set *r_set, uint32_t idx, int32_t type) {
    reg *temp = r_set->r[idx];

    if (type == REG_M_SORT_UP) {
        for (uint32_t i=idx; i<REG_SIZE-1; ++i) {
            r_set->r[i] = r_set->r[i+1];
        }

        r_set->r[REG_SIZE-1] = temp;
    } else {
        for (uint32_t i=idx; i>0; --i) {
            r_set->r[i] = r_set->r[i-1];
        }

        r_set->r[0] = temp;
    }
}

reg_allocator reg_m_get(reg_set *r_set, addr_t addr, bool do_sort) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i]->addr == addr) {
            if (do_sort) {
                reg_m_sort(r_set, i, REG_M_SORT_UP);
            }

            return (reg_allocator){r_set->r[REG_SIZE-1], i, true};
        }
    }

    if (do_sort) {
        reg_m_sort(r_set, 0, REG_M_SORT_UP);
    }
    return (reg_allocator){r_set->r[REG_SIZE-1], 0, false};
}

reg_allocator reg_m_LRU(reg_set *r_set, bool do_sort) {
    if (do_sort) {
        reg_m_sort(r_set, 0, REG_M_SORT_UP);
    }

    return (reg_allocator){r_set->r[REG_SIZE-1], 0, false};
}

void reg_m_drop_addr(reg_set *r_set, addr_t addr) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i]->addr == addr) {
            r_set->r[i]->addr = ADDR_UNDEF;
            r_set->r[i]->flags = REG_NO_FLAGS;
            reg_m_sort(r_set, i, REG_M_SORT_DOWN);

            return;
        }
    }
}
