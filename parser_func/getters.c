#include "getters.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static symbol_table *s_table = NULL;
static reg_set *r_set = NULL;

symbol_table * get_symbol_table() {
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

static void free_symbol(symbol *sym) {
    free((void *)(sym->identifier));
    if (sym->flags & SYMBOL_IS_ARRAY) {
        array_value *arr_val = sym->consts.arr_value;
        while (arr_val) {
            array_value *to_free = arr_val;
            arr_val = arr_val->next;
            mpz_clear(to_free->value);
            free(to_free);
        }
    } else {
        mpz_clear(sym->consts.value);
    }
}

void free_symbol_table() {
    if (s_table) {
        symbol *sym = NULL;
        while ((sym = VECTOR_POP(s_table->v, POP)) != s_table->v._mem_ptr) {
            free_symbol(sym);
        }
        free_symbol(sym);

        free(s_table->v._mem_ptr);
        free(s_table);
    }
}

reg_set * get_reg_set() {
    if (r_set == NULL) {
        reg_set const new_r_set = reg_m_create();

        r_set = malloc(sizeof(reg_set));
        if (!r_set) {
            fprintf(stderr, "[GETTERS]: Couldn't allocate memory for register set!\n");
            exit(EXIT_FAILURE);
        }
        *r_set = new_r_set;
    }

    return r_set;
}

void free_reg_set() {
    if (r_set) {
        for (int32_t i=0; i<REG_SIZE; ++i) {
            free(r_set->r[i]);
        }
        free(r_set);
    }
}
