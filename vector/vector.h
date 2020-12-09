#ifndef VECTOR_H
#define VECTOR_H

/**
 * 
 * MACRO for vector_pop: address
 * 
 * ARGUMENTS: vector, macro takes address iself
*/
#ifndef VECTOR_POP
#define VECTOR_POP(vec) vector_pop(&vec)
#endif

/**
 * 
 * MACRO for vector_add: void
 * 
 * ARGUMENTS: vector, element
 * 
*/
#ifndef VECTOR_ADD
#define VECTOR_ADD(vec, new_element) vector_add(&vec, &new_element)
#endif

/**
 * 
 * MACRO for vector_find: void
 * 
 * ARGUMENTS: vector, element_to_find (instance of element with specified member values to find), comparator_eq (function identifier)
 * 
*/
#ifndef VECTOR_FIND
#define VECTOR_FIND(vec, element_to_find, comparator_eq) vector_find(&vec, &element_to_find, &comparator_eq)
#endif

#include <stdlib.h>
#include <stdalign.h>
#include <stdbool.h>

typedef struct {
    void *_mem_ptr;
    const size_t _element_size;
    const size_t _alignment;
    size_t used_size;
    size_t max_size;
} vector;

typedef struct {
    void *element_ptr;
    size_t idx;
} vector_element;

typedef int (*comp_eq)(void *, void *);

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
 * ARGUMENTS: vector *, bool do_pop - if true then element is popped from vector
 * RETURN: address of last element in vector 
*/
void * vector_pop(vector *vec, bool do_pop);

/**
 * 
 * Find element with comparator_eq function of type comp_eq
 * comparator_eq should return 1 when element in vector == element_to_find by definition
 * 
 * RETURN: vector_element: address of found element and it's index in vector. ON failed search address is NULL
*/
vector_element vector_find(vector *vec, void *element_to_find, comp_eq comparator_eq);

#endif
