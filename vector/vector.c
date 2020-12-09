#include <stdio.h>
#include <string.h>

#include "vector.h"

vector vector_create(size_t element_size, size_t alignment, size_t size) {
    if (!size) {
        size = 1;
    }

    vector vec = {
        ._mem_ptr = NULL,
        ._element_size = element_size,
        ._alignment = alignment,
        .used_size = 0,
        .max_size = size
    };

    if (!(vec._mem_ptr = aligned_alloc(alignment, size*element_size))) {
        fprintf(stderr, "[vector]: Memory allocation error!\n");
        exit(EXIT_FAILURE);
    }

    return vec;
}

void vector_add(vector *vec, void *new_element) {
    if (vec->used_size < vec->max_size) {
        memcpy(vec->_mem_ptr + vec->used_size * vec->_element_size, new_element, vec->_element_size);
        ++(vec->used_size);
    } else {
        void *new_mem;
        if (!(new_mem = aligned_alloc(vec->_alignment, 2 * vec->max_size * vec->_element_size))) {
            fprintf(stderr, "[vector]: Memory reallocation error!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(new_mem, vec->_mem_ptr, vec->max_size * vec->_element_size);
        vec->max_size = 2 * vec->max_size;

        free(vec->_mem_ptr);
        vec->_mem_ptr = new_mem;

        return vector_add(vec, new_element);
    }
}

void * vector_pop(vector *vec, bool do_pop) {
    if (vec->used_size) {
        if (do_pop) {
            --(vec->used_size);
        }

        return vec->_mem_ptr + vec->used_size * vec->_element_size;
    }

    fprintf(stderr, "[vector]: POP of empty vector!\n"); // It can work this way :)
    return vec->_mem_ptr;
}

vector_element vector_find(vector *vec, void *element_to_find, comp_eq comparator_eq) {
    for (int32_t i=0; i<vec->used_size; ++i) {
        if (comparator_eq(element_to_find, vec->_mem_ptr + i * vec->_element_size)) {
            return (vector_element){vec->_mem_ptr + i * vec->_element_size, i};
        }
    }

    return (vector_element){NULL, 0};
}
