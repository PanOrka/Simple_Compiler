#include "i_level.h"

#include <stdlib.h>
#include <stdio.h>

static i_level *start = NULL;
static i_level *end = NULL;

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
