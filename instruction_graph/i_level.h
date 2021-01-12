#ifndef I_LEVEL_H
#define I_LEVEL_H

#include <stdbool.h>

#ifndef i_POP
#define i_POP 1
#endif

#ifndef i_NOPOP
#define i_NOPOP 0
#endif

#include "i_graph.h"
#include "../register_machine/reg_m.h"
#include "../parser_func/expressions.h"

#include <stdint.h>

typedef struct i_level i_level;

typedef instruction_type branch_type;

struct i_level {
    i_level *prev;
    uint64_t i_num;
    int64_t *reserved_jmp;
    reg_snapshot r_snap;
    void *payload;
    branch_type type;
};

/**
 * 
 * Add new i_level
 * 
*/
void i_level_add(branch_type type);

void i_level_add_branch_eval(branch_type type, bool have_mpz, void *payload);

i_level i_level_pop_branch_eval(bool pop);

/**
 * 
 * Pop last
 * 
 * ARGUMENTS: bool whether POP should delete element from list
 * RETURN: Copy of last value in list
*/
branch_type i_level_pop(bool pop);

/**
 * 
 * Check whether list is empty
 * 
 * RETURN: Bool <- is list empty
*/
bool i_level_is_empty();
bool i_level_is_empty_eval();

#endif
