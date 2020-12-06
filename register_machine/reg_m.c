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
