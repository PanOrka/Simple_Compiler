#include "reg_m.h"

reg_set reg_m_create() {
    char reg_id_arr[REG_SIZE] = REG_ID_SET;
    reg_set regs;

    for (int32_t i=0; i<REG_SIZE; ++i) {
        regs.r[i].addr = ADDR_UNDEF;
        regs.r[i].id = reg_id_arr[i];
    }

    return regs;
}

void reg_m_sort(uint32_t idx, reg_set *r_set) {
    if (idx < REG_SIZE-1) {
        reg temp = r_set->r[idx];

        for (uint32_t i=idx; i<REG_SIZE-1; ++i) {
            r_set->r[i] = r_set->r[i+1];
        }

        r_set->r[REG_SIZE-1] = temp;
    }
}

reg_allocator reg_m_get(uint64_t addr, reg_set *r_set) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i].addr == addr) {
            reg_m_sort(i, r_set);

            return (reg_allocator){&(r_set->r[REG_SIZE-1]), i, true};
        }
    }

    reg_m_sort(0, r_set);
    return (reg_allocator){&(r_set->r[REG_SIZE-1]), 0, false};
}

reg_allocator reg_m_LRU(reg_set *r_set) {
    reg_m_sort(0, r_set);

    return (reg_allocator){&(r_set->r[REG_SIZE-1]), 0, false};
}
