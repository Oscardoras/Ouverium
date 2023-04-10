#ifndef __GC_H__
#define __GC_H__

#include <stdbool.h>


/**
 * Type of a function iterator called by the garbage collector.
*/
typedef void (*__GC_Iterator)(void*);
void __GC_NULL_iterator(void*);

/**
 * Represents a function context.
*/
typedef struct __GC_Context {
    struct __GC_Context* next;
    __GC_Iterator iterator;
} __GC_Context;

/**
 * Represents an element of the linked list of elements in the heap.
*/
typedef struct __GC_Element {
    struct __GC_Element* next;
    bool iterated;
} __GC_Element;

void __GC_init(__GC_Iterator gc_global_context_iterator);
void* __GC_alloc_object(unsigned long size);
void __GC_collect(void);
void __GC_end(void);


#endif
