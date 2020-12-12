#include "getters.h"

#include <string.h>

symbol_table * get_symbol_table() {
    static symbol_table *s_table = NULL;
    if (s_table == NULL) {
        symbol_table const new_s_table = symbol_table_create();
        s_table = malloc(sizeof(symbol_table));

        memcpy(s_table, &new_s_table, sizeof(symbol_table));
    }

    return s_table;
}

reg_set * get_reg_set() {
    static reg_set *r_set = NULL;
    if (r_set == NULL) {
        reg_set const new_r_set = reg_m_create();

        r_set = malloc(sizeof(reg_set));
        *r_set = new_r_set;
    }

    return r_set;
}
