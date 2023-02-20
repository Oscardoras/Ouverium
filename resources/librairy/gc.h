#ifndef __GC_H__
#define __GC_H__

#include <stdbool.h>


/**
 * Represents a data.
*/
typedef union __Data {
    void* ptr;
    long i;
    double f;
    char c;
    bool b;
} __Data;


// Garbage Collector.

/**
 * Type of a function iterator called by the garbage collector.
*/
typedef void (*__GC_Iterator)(void*);

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
__GC_Element* __GC_alloc_object(unsigned long size);
void __GC_collect(void);
void __GC_end(void);


/**
 * Type of a function iterator called to access to an element from an array.
*/
typedef __UnknownData (*__Array_Iterator)(void* array, unsigned long i);

/**
 * Contains information to manage a data type.
*/
typedef struct __VirtualTable {
    __GC_Iterator gc_iterator;
    __Array_Iterator array_iterator;
} __VirtualTable;

/**
 * Represents a data which the real type is unknown.
*/
typedef struct __UnknownData {
    __VirtualTable* virtual_table;
    __Data data;
} __UnknownData;


/**
 * Represents a reference to a data.
*/
typedef struct __Reference {
    enum {
        DATA,
        REFERENCE,
        PROPERTY,
        ARRAY,
    } type;
    union {
        __UnknownData data;
        __UnknownData* reference;
        struct {
            __UnknownData object;
            __UnknownData property;
        } property;
        struct {
            __UnknownData array;
            unsigned long i;
        } array;
    };
} __Reference;

__UnknownData __Reference_get(__Reference reference);


typedef bool (*__FunctionFilter)(__GC_Context* context, __Reference args);
typedef __Reference (*__FunctionBody)(__GC_Context* context, __Reference args);

/**
 * Represents a function in a function linked list.
*/
typedef struct __Function {
    __Function* next;
    __FunctionFilter filter;
    __FunctionBody body;
    unsigned short references_size;
    __Reference* references;
} __Function;

__Reference __Function_eval(__GC_Context* context, __Function* function, __Reference args);


#endif
