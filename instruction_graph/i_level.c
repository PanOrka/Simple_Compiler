#include "i_level.h"
#include "../vector/vector.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static i_level *start = NULL;
static i_level *end = NULL;

static i_level *start_eval = NULL;
static i_level *end_eval = NULL;

static vector jump_vec;

void get_jump_vec(vector *j_vec) {
    memcpy(j_vec, &jump_vec, sizeof(vector));
}

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

#include "instructions/asm_fprintf.h"
#include "../parser_func/getters.h"

static void i_level_add_eval(branch_type type) {
    i_level *new_element = malloc(sizeof(i_level));
    if (new_element) {
        new_element->type = type;

        if (start_eval) {
            new_element->prev = end_eval;
        } else {
            new_element->prev = NULL;
            start_eval = new_element;
        }

        end_eval = new_element;
    } else {
        fprintf(stderr, "[I_LEVEL]: Couldn't allocate memory for I_LVL node!\n");
        exit(EXIT_FAILURE);
    }
}

void i_level_add_branch_eval(branch_type type, bool have_mpz, void *payload) {
    if (jump_vec._mem_ptr == NULL) {
        vector jump_vec_init = vector_create(sizeof(int64_t), alignof(int64_t), JUMP_VEC_INIT_SIZE);
        memcpy(&jump_vec, &jump_vec_init, sizeof(vector));
    }

    i_level_add_eval(type);
    end_eval->i_num = asm_get_i_num();
    end_eval->r_snap = reg_m_snapshot(get_reg_set(), have_mpz);
    end_eval->payload = payload;

    int64_t val = 0;
    VECTOR_ADD(jump_vec, val);
    end_eval->reserved_jmp_idx = jump_vec.used_size - 1;
}

static void i_level_pop_eval(bool pop) {
    if (!end_eval) {
        fprintf(stderr, "[I_LEVEL]: List is empty!\n");
        exit(EXIT_FAILURE);
    }

    branch_type temp = end_eval->type;
    if (pop) {
        i_level *to_free = end_eval;
        if (!(end_eval->prev)) {
            start_eval = NULL;
        }

        end_eval = end_eval->prev;
        free(to_free);
    }
}

i_level i_level_pop_branch_eval(bool pop) {
    i_level ret_pop = *end_eval;
    i_level_pop_eval(pop);

    return ret_pop;
}

bool i_level_is_empty_eval() {
    return end_eval == NULL;
}

void i_level_set_reserved_jump(idx_t vec_idx, int64_t value) {
    int64_t * const jmp_ptr = vector_get(&jump_vec, vec_idx);
    *jmp_ptr = value;
}
