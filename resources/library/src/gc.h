#ifndef __GC_H__
#define __GC_H__

#include "include.h"


#ifdef __cplusplus
extern "C" {
#endif

    typedef struct __GC_Element {
        struct __GC_Element* next;
        __VirtualTable* vtable;
        bool iterated;
    } __GC_Element;

    typedef struct __GC_Roots {
        struct __GC_Roots* next;
        size_t capacity;
        size_t size;
        __Reference* tab[];
    } __GC_Roots;

    void __GC_init_roots(void);
    void __GC_add_reference(__Reference* reference);
    void __GC_move_reference(__Reference* reference);
    void __GC_remove_reference();
    void __GC_end_roots(void);

    void __Reference_move_data(__Reference* dest, __Reference* src);
    void __Reference_free_data(__Reference* reference);

#ifdef __cplusplus
}
#endif


#endif
