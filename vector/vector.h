#ifndef VECTOR_H
#define VECTOR_H

/**
 * 
 * MACRO for vector_pop: address -> element
 * 
*/
#ifndef VECTOR_POP
#define VECTOR_POP(vec) *(vector_pop(vec))
#endif

#include <stdlib.h>
#include <stdalign.h>

typedef struct {
    void *_mem_ptr;
    size_t _element_size;
    size_t _alignment;
    size_t used_size;
    size_t max_size;
} vector;

/**
 * 
 * Allocate vector with given size of element, alignment and size
 * size = # of elements
 * 
 * RETURN: copy of vector with aligned memory
*/
vector vector_create(size_t element_size, size_t alignment, size_t size);

/**
 * 
 * Add element to vector
 * 
 * Does copy of memory from void *new_element with memcopy() to first free space in vector
 * 
*/
void vector_add(vector *vec, void *new_element);

/**
 * 
 * Pops element from vector
 * 
 * RETURN: address of last element in vector 
*/
void * vector_pop(vector *vec);

#endif
