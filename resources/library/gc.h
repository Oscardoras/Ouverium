#ifndef __GC_H__
#define __GC_H__

#include <stdbool.h>


struct __UnknownData;

/**
 * Type of a function iterator called by the garbage collector.
*/
typedef void (*__GC_Iterator)(void*);
void __GC_NULL_iterator(void*);

/**
 * Represents a function context.
*/
typedef struct __GC_Context {
    struct __GC_Context* parent;
    __GC_Iterator iterator;
} __GC_Context;

/**
 * Represents an element of the linked list of elements in the heap.
*/
typedef struct __GC_Element {
    struct __GC_Element* next;
    bool iterated;
} __GC_Element;

#define __GC_begin_function(iterator) \
    __GC_Context __GC_context = { .parent = __GC_contexts, .iterator = iterator }; \
    __GC_contexts = &context;

#define __GC_end_function() \
    __GC_contexts = context->parent;

void __GC_init();
void* __GC_alloc_object(size_t size);
void __GC_collect(void);
void __GC_end(void);

extern _Thread_local __GC_Context* __GC_contexts;


#endif
