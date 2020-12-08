#include <stdio.h>

#include "vector.h"

vector vector_create(size_t element_size, size_t alignment, size_t size) {
    vector vec = {
        NULL,
        element_size,
        alignment,
        0,
        size
    };

    if (!(vec._mem_ptr = aligned_alloc(alignment, size*element_size))) {
        fprintf(stderr, "[vector]: Memory allocation error!\n");
        exit(EXIT_FAILURE);
    }

    return vec;
}

void vector_add(vector *vec, void *new_element) {
    
}
