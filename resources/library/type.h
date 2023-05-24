#ifndef __TYPE_H__
#define __TYPE_H__

#include "function.h"


#ifdef __cplusplus

template<typename T>
constexpr size_t index;

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
    static size_t constexpr get_size() {
        return 0;
    }
    template<typename C, typename... Cs>
    static constexpr size_t get_size() {
        constexpr size_t a = get_size<Cs...>();
        constexpr size_t b = index<C>;
        if constexpr (a > b)
            return a;
        else
            return b;
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

    inline static VirtualTable<get_size<Components...>()> vtable;

    static_assert(create_vtable(&vtable));

};

typedef Type<void, int> Integer;

void tes() {
    Integer i;
}

#endif


#endif
