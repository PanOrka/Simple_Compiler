#include "getters.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

symbol_table * get_symbol_table() {
    static symbol_table *s_table = NULL;
    if (s_table == NULL) {
        symbol_table const new_s_table = symbol_table_create();
        s_table = malloc(sizeof(symbol_table));
        if (!s_table) {
            fprintf(stderr, "[GETTERS]: Couldn't allocate memory for symbol table!\n");
            exit(EXIT_FAILURE);
        }

        memcpy(s_table, &new_s_table, sizeof(symbol_table));
    }

    return s_table;
}

static void print_regs(reg_set *r_set) {
    fprintf(stdout, "\n");
    for (int32_t i=0; i<REG_SIZE; ++i) {
        fprintf(stdout, "REG: %c, ADDR: %lu\n", r_set->r[i]->id, r_set->r[i]->addr);
    }
    fprintf(stdout, "REG: %c, ADDR: %lu\n", r_set->stack_ptr.id, r_set->stack_ptr.addr);
}

reg_set * get_reg_set() {
    static reg_set *r_set = NULL;
    if (r_set == NULL) {
        reg_set const new_r_set = reg_m_create();

        r_set = malloc(sizeof(reg_set));
        if (!r_set) {
            fprintf(stderr, "[GETTERS]: Couldn't allocate memory for register set!\n");
            exit(EXIT_FAILURE);
        }
        *r_set = new_r_set;
    }

    // for debug
    print_regs(r_set);

    return r_set;
}
