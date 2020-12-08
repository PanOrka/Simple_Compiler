#include <stdio.h>
#include <string.h>

#include "vector.h"

vector vector_create(size_t element_size, size_t alignment, size_t size) {
    if (!size) {
        size = 1;
    }

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
    if (vec->used_size < vec->max_size) {
        memcpy(vec->_mem_ptr + vec->used_size * vec->_element_size, new_element, vec->_element_size);
        ++(vec->used_size);
    } else {
        vec->max_size = 2 * vec->max_size;

        void *new_mem;
        if (!(new_mem = aligned_alloc(vec->_alignment, vec->max_size * vec->_element_size))) {
            fprintf(stderr, "[vector]: Memory reallocation error!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(new_mem, vec->_mem_ptr, vec->max_size * vec->_element_size);

        free(vec->_mem_ptr);
        vec->_mem_ptr = new_mem;

        return vector_add(vec, new_element);
    }
}

typedef struct {
    int a;
    short b;
} test;

int main() {
    vector v = vector_create(sizeof(test), alignof(test), 0);
    test a = {1, 1};
    vector_add(&v, &a);
    a.a = 2, a.b = 2;
    vector_add(&v, &a);

    for (int i=0; i<2; ++i) {
        printf("%d, %d\n", ((test *)v._mem_ptr)[i].a, ((test *)v._mem_ptr)[i].b);
    }
}
