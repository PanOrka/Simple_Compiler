#ifndef I_LEVEL_H
#define I_LEVEL_H

#include <stdbool.h>

#include "i_graph.h"

#ifndef i_POP
#define i_POP 1
#endif

#ifndef i_NOPOP
#define i_NOPOP 0
#endif

typedef struct i_level i_level;

typedef instruction_type branch_type;

struct i_level {
    i_level *prev;
    branch_type type;
};

/**
 * 
 * Add new i_level
 * 
*/
void i_level_add(branch_type type);

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

#endif
