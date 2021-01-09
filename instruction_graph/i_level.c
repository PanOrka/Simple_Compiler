#include "i_level.h"
#include "../vector/vector.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static i_level *start = NULL;
static i_level *end = NULL;

static vector jump_vec;

void i_level_add(branch_type type) {
    i_level *new_element = malloc(sizeof(i_level));
    if (new_element) {
        new_element->type = type;

        if (start) {
            new_element->prev = end;
        } else {
            new_element->prev = NULL;
            start = new_element;
        }

        end = new_element;
    } else {
        fprintf(stderr, "[I_LEVEL]: Couldn't allocate memory for I_LVL node!\n");
        exit(EXIT_FAILURE);
    }
}

#include "instructions/asm_fprintf.h"
#include "../parser_func/getters.h"

void i_level_add_branch_eval(branch_type type) {
    if (jump_vec._mem_ptr == NULL) {
        vector jump_vec_init = vector_create(sizeof(int64_t), alignof(int64_t), 16);
        memcpy(&jump_vec, &jump_vec_init, sizeof(vector));
    }

    i_level_add(type);
    end->i_num = asm_get_i_num();
    end->r_snap = reg_m_snapshot(get_reg_set());

    int64_t val = 0;
    VECTOR_ADD(jump_vec, val);
    end->reserved_jmp = VECTOR_POP(jump_vec, NO_POP);
}

branch_type i_level_pop(bool pop) {
    if (!end) {
        fprintf(stderr, "[I_LEVEL]: List is empty!\n");
        exit(EXIT_FAILURE);
    }

    branch_type temp = end->type;
    if (pop) {
        i_level *to_free = end;        
        if (!(end->prev)) {
            start = NULL;
        }

        end = end->prev;
        free(to_free);
    }

    return temp;
}

bool i_level_is_empty() {
    return end == NULL;
}
