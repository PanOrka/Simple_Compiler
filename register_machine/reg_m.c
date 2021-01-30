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

static void print_regs(reg_set *r_set) {
    fprintf(stdout, "\n");
    for (int32_t i=0; i<REG_SIZE; ++i) {
        fprintf(stdout, "REG: %c, ADDR: %lu\n", r_set->r[i]->id, r_set->r[i]->addr);
    }
    fprintf(stdout, "REG: %c, ADDR: %lu\n", r_set->stack_ptr.id, r_set->stack_ptr.addr);
}

reg_allocator reg_m_get(reg_set *r_set, addr_t addr, bool do_sort) {
    int32_t idx = 0;
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i]->addr == addr) {
            if (do_sort) {
                reg_m_sort(r_set, i, REG_M_SORT_UP);
                idx = REG_SIZE - 1;
            } else {
                idx = i;
            }

            return (reg_allocator){r_set->r[idx], i, true};
        }
    }

    if (do_sort) {
        reg_m_sort(r_set, 0, REG_M_SORT_UP);
        idx = REG_SIZE - 1;
    }

    // for debug
    //print_regs(r_set);

    return (reg_allocator){r_set->r[idx], 0, false};
}

reg_allocator reg_m_LRU(reg_set *r_set, bool do_sort) {
    int32_t idx = 0;
    if (do_sort) {
        reg_m_sort(r_set, 0, REG_M_SORT_UP);
        idx = REG_SIZE - 1;
    }

    // for debug
    //print_regs(r_set);

    return (reg_allocator){r_set->r[idx], 0, false};
}

void reg_m_drop_addr(reg_set *r_set, addr_t addr) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i]->addr == addr) {
            r_set->r[i]->addr = ADDR_UNDEF;
            r_set->r[i]->flags = REG_NO_FLAGS;
            reg_m_sort(r_set, i, REG_M_SORT_DOWN);

            // for debug
            //print_regs(r_set);

            return;
        }
    }
}

void reg_m_promote(reg_set *r_set, addr_t addr) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i]->addr == addr) {
            reg_m_sort(r_set, i, REG_M_SORT_UP);

            // for debug
            //print_regs(r_set);

            return;
        }
    }

    fprintf(stderr, "[REG_M]: Promotion of non-existent address!\n");
    exit(EXIT_FAILURE);
}

void stack_ptr_set_mpz_to_current_value(mpz_t dest);
void stack_ptr_set_mpz(mpz_t src);
bool stack_ptr_mpz_initialized();
bool stack_ptr_initialized();
void stack_ptr_set_init_flag(bool flag);

void val_gen_set_mpz_to_current_value(mpz_t dest);
void val_gen_set_mpz(mpz_t src);
bool val_gen_mpz_initialized();

reg_snapshot reg_m_snapshot(reg_set *r_set, bool have_mpz) {
    reg_snapshot r_snap;
    for (int32_t i=0; i<REG_SIZE; ++i) {
        r_snap.r[i] = *(r_set->r[i]);
    }

    r_snap.have_mpz = have_mpz;
    r_snap.stack_ptr_init = stack_ptr_initialized();
    if (have_mpz) {
        mpz_init(r_snap.stack_ptr_value);
        if (r_snap.stack_ptr_init_mpz = stack_ptr_mpz_initialized()) {
            stack_ptr_set_mpz_to_current_value(r_snap.stack_ptr_value);
        }

        mpz_init(r_snap.val_gen_value);
        if (r_snap.val_gen_init_mpz = val_gen_mpz_initialized()) {
            val_gen_set_mpz_to_current_value(r_snap.val_gen_value);
        }
    }

    return r_snap;
}

void reg_m_apply_snapshot(reg_set *r_set, reg_snapshot r_snap) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        *(r_set->r[i]) = r_snap.r[i];
    }

    stack_ptr_set_init_flag(r_snap.stack_ptr_init);
    if (r_snap.have_mpz) {
        if (r_snap.stack_ptr_init_mpz) {
            stack_ptr_set_mpz(r_snap.stack_ptr_value);
        }

        if (r_snap.val_gen_init_mpz) {
            val_gen_set_mpz(r_snap.val_gen_value);
        }

        mpz_clear(r_snap.stack_ptr_value);
        mpz_clear(r_snap.val_gen_value);
    }
}

void reg_m_sort_by_snapshot(reg_set *r_set, reg r[REG_SIZE]) {
    reg *temp_regs[REG_SIZE] = {0};
    for (int32_t i=0; i<REG_SIZE; ++i) {
        temp_regs[i] = reg_m_get_by_id(r_set, r[i].id);
    }

    for (int32_t i=0; i<REG_SIZE; ++i) {
        r_set->r[i] = temp_regs[i];
    }
}

reg * reg_m_get_by_id(reg_set *r_set, char id) {
    for (int32_t i=0; i<REG_SIZE; ++i) {
        if (r_set->r[i]->id == id) {
            return r_set->r[i];
        }
    }

    return NULL;
}
