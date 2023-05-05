#ifndef __GC_H__
#define __GC_H__

#include "reference.h"


#define ROOTS_CAPACITY 128

void __GC_NULL_iterator(void*);

/**
 * Represents an element of the linked list of elements in the heap.
*/
typedef struct __GC_Element {
    struct __GC_Element* next;
    bool iterated;
} __GC_Element;

/**
 * Represents a reference to a data.
*/
typedef struct __GC_Reference {
    enum {
        NONE,
        DATA,
        SYMBOL,
        PROPERTY,
        ARRAY,
        TUPLE
    } type;
    union {
        struct {
            size_t size;
            struct __GC_Reference* next;
        } none;

        __UnknownData data;
        __UnknownData* symbol;
        struct {
            __UnknownData parent;
            __VirtualTable* virtual_table;
            void* property;
        } property;
        struct {
            __UnknownData array;
            size_t i;
        } array;
        struct {
            struct __GC_Reference *references;
            size_t size;
        } tuple;
    };
} __GC_Reference;

typedef struct __GC_Roots {
    struct __GC_Roots* next;
    size_t capacity;
    __GC_Reference* free_list;
} __GC_Roots;

void __GC_init();
__GC_Roots* __GC_init_roots(size_t c);
__GC_Reference *__GC_alloc_references(size_t n);
void __GC_free_reference(__GC_Reference* reference);
void* __GC_alloc_object(size_t size);
void __GC_iterate(__GC_Iterator iterator, void* object);
void __GC_collect(void);
void __GC_end(void);


#endif
