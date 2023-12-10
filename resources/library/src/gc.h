#ifndef __GC_H__
#define __GC_H__

#include "include.h"


#ifdef __cplusplus
extern "C" {
#endif

#define ROOTS_CAPACITY 128

    typedef struct Ov_GC_Element {
        struct Ov_GC_Element* next;
        Ov_VirtualTable* vtable;
        bool iterated;
    } Ov_GC_Element;

    typedef struct Ov_GC_Reference {
        enum {
            NONE = 0,
            DATA,
            SYMBOL,
            PROPERTY,
            ARRAY,
            TUPLE
        } type;
        union {
            struct {
                size_t size;
                struct Ov_GC_Reference* next;
            } none;

            Ov_UnknownData data;
            Ov_UnknownData* symbol;
            struct {
                Ov_UnknownData parent;
                Ov_VirtualTable* vtable;
                unsigned int hash;
            } property;
            struct {
                Ov_UnknownData array;
                size_t i;
            } array;
            struct {
                Ov_VirtualTable* vtable;
                struct Ov_GC_Reference* references;
                size_t size;
            } tuple;
        };
    } Ov_GC_Reference;

    typedef struct Ov_GC_Roots {
        struct Ov_GC_Roots* next;
        size_t capacity;
        Ov_GC_Reference* free_list;
    } Ov_GC_Roots;

    Ov_GC_Roots* Ov_GC_init_roots(size_t c);
    Ov_GC_Reference* Ov_GC_alloc_references(size_t n);
    void Ov_GC_free_reference(Ov_GC_Reference* reference);

    struct Ov_FunctionCapture Ov_GC_reference_to_capture(Ov_GC_Reference* reference);
    Ov_Reference_Owned Ov_GC_capture_to_reference(struct Ov_FunctionCapture capture);

    void Ov_init_functions();

#ifdef __cplusplus
}
#endif


#endif
