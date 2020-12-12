#include "reg_m.h"

reg_set reg_m_create() {
    char reg_id_arr[REG_SIZE] = REG_ID_SET;
    reg_set regs;

    for (int32_t i=0; i<REG_SIZE; ++i) {
        regs.r[i].addr = ADDR_UNDEF;
        regs.r[i].id = reg_id_arr[i];
        regs.r[i].flags = REG_NO_FLAGS;
    }

    return regs;
}

#ifndef REG_M_SORT_UP
#define REG_M_SORT_UP 1
#endif

#ifndef REG_M_SORT_DOWN
#define REG_M_SORT_DOWN -1
#endif

void reg_m_sort(reg_set *r_set, uint32_t idx, int32_t type) {
    reg temp = r_set->r[idx];

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

reg_allocator reg_m_get(reg_set *r_set, addr_t addr) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i].addr == addr) {
            reg_m_sort(r_set, i, REG_M_SORT_UP);

            return (reg_allocator){&(r_set->r[REG_SIZE-1]), i, true};
        }
    }

    reg_m_sort(r_set, 0, REG_M_SORT_UP);
    return (reg_allocator){&(r_set->r[REG_SIZE-1]), 0, false};
}

reg_allocator reg_m_LRU(reg_set *r_set) {
    reg_m_sort(r_set, 0, REG_M_SORT_UP);

    return (reg_allocator){&(r_set->r[REG_SIZE-1]), 0, false};
}

void reg_m_drop_addr(reg_set *r_set, addr_t addr) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i].addr == addr) {
            r_set->r[i].addr = ADDR_UNDEF;
            r_set->r[i].flags = REG_NO_FLAGS;
            reg_m_sort(r_set, i, REG_M_SORT_DOWN);

            return;
        }
    }
}
