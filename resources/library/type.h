#ifndef __TYPE_H__
#define __TYPE_H__

#include "function.h"


#ifdef __cplusplus

template<typename T>
constexpr size_t index = 0;

template<typename ArrayType, typename... Components>
class Type: public Components... {

private:

    template<size_t size>
    struct VirtualTable {
        __VirtualTable_Info info;
        __VirtualTable_TabElement tab[size];
    };

    void iterate() {}
    template<typename C, typename... Cs>
    void iterate() {
        __GC_iterate(static_cast<C*>(this));
        iterate<Cs...>();
    }

    static constexpr Type<ArrayType, Components...> object = Type<ArrayType, Components...>();

    static constexpr void add_vtable(__VirtualTable* vtable) {}
    template<typename C, typename... Cs>
    static constexpr void add_vtable(__VirtualTable* vtable) {
        vtable->tab[index<C>].offset = ptrdiff_t(static_cast<C*>(&object)) - ptrdiff_t(&object);
        add_vtable<Cs...>();
    }
    static constexpr size_t max() {
        return 0;
    }
    template<typename A, typename... B>
    static constexpr size_t max(A a, B... b...) {
        size_t c = max(b...);
        if (a > c)
            return a;
        else
            return c;
    }
    static constexpr bool create_vtable(__VirtualTable* vtable) {
        vtable->size = sizeof(Type<ArrayType, Components...>);
        vtable->gc_iterator = __GC_iterator;
        vtable->array.vtable = ArrayType::vtable;
        vtable->array.offset = 0;
        vtable->function_offset
        add_vtable<Components>();
        return true;
    }

protected:

    static void __GC_iterator(Type<ArrayType, Components...>* object) {
        object.iterate();
    }

public:

    inline static VirtualTable<max(index<Components>...)> vtable;

    static_assert(create_vtable((__VirtualTable*) (void*) &vtable));

};

struct Compo {

};
template<>
constexpr size_t index<Compo> = 10;

typedef Type<void, Compo> Integer;

void tes() {
    Integer i;
}

#endif


#endif
