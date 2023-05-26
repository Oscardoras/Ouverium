#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#include <array>
#include <vector>

#include "virtual_tables.h"


template<typename T, __VirtualTable* vtable = &T::vtable>
class Array : protected __Array {

public:

    static constexpr size_t index = __Component___Array_index;

    Array(size_t capacity = 0, size_t size = 0) {
        __Array::capacity = capacity;
        __Array::size = size;
        tab = calloc(capacity, vtable->info.size);
    }

    operator __ArrayInfo() const {
        return __ArrayInfo{
            .vtable = vtable,
            .array = this
        };
    }

    size_t size() const {
        return size;
    }

    void set_size(size_t const size) {
        return __Array_set_size(*this, size);
    }

    size_t capacity() const {
        return capacity;
    }

    void set_capacity(size_t const size) {
        return __Array_set_capacity(*this, size);
    }

    bool empty() const {
        return size() == 0;
    }

    T& operator[](size_t const i) const {
        return *__Array_get(*this, i);
    }

    class iterator {

        Array<T>& array;
        size_t i;

    public:

        iterator(Array<T>& array, size_t const i) :
            array{array}, i{ i } {}

        size_t operator++() {
            return i++;
        }

        size_t operator++(int) {
            return ++i;
        }

        size_t operator--() {
            return i--;
        }

        size_t operator--(int) {
            return --i;
        }

        bool operator==(iterator const& it) const {
            return it.i == i;
        }

        bool operator!=(iterator const& it) const {
            return !(it == *this);
        }

        T& operator*() const {
            return array[i];
        }

    };

    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, size());
    }

};

class ArrayInfo {

protected:

    __ArrayInfo array;

public:

    ArrayInfo(__ArrayInfo array) :
        array{ array } {}

    operator __ArrayInfo () const {
        return array;
    }

    size_t size() const {
        return array.array->size;
    }

    void set_size(size_t const size) {
        return __Array_set_size(array, size);
    }

    size_t capacity() const {
        return array.array->capacity;
    }

    void set_capacity(size_t const size) {
        return __Array_set_capacity(array, size);
    }

    bool empty() const {
        return size() == 0;
    }

    template<typename T>
    T& operator[](size_t const i) const {
        return *__Array_get(*this, i);
    }

    class iterator {

        ArrayInfo& array;
        size_t i;

    public:

        iterator(ArrayInfo& array, size_t const i) :
            array{ array }, i{ i } {}

        size_t operator++() {
            return i++;
        }

        size_t operator++(int) {
            return ++i;
        }

        size_t operator--() {
            return i--;
        }

        size_t operator--(int) {
            return --i;
        }

        bool operator==(iterator const& it) const {
            return it.i == i;
        }

        bool operator!=(iterator const& it) const {
            return !(it == *this);
        }

        template<typename T>
        T& operator*() const {
            return array.operator[]<T>(i);
        }

    };

    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, size());
    }

};

class UnknownData {

protected:

    __UnknownData data;

public:

    UnknownData(__UnknownData const& data) :
        data{ data } {}

    UnknownData(__VirtualTable* vtable, void* d, ...) :
        data{ __UnknownData_from_data(vtable, d) } {}

    UnknownData(__VirtualTable* vtable, void* ptr) :
        data{ __UnknownData_from_ptr(vtable, ptr) } {}

    UnknownData(long i) {
        data.virtual_table = &__VirtualTable_Int;
        data.data.i = i;
    }
    UnknownData(double f) {
        data.virtual_table = &__VirtualTable_Float;
        data.data.f = f;
    }
    UnknownData(char c) {
        data.virtual_table = &__VirtualTable_Char;
        data.data.c = c;
    }
    UnknownData(bool b) {
        data.virtual_table = &__VirtualTable_Bool;
        data.data.b = b;
    }
    template<class T>
    UnknownData(T* ptr) {
        data.virtual_table = &T::vtable;
        data.data.b = ptr;
    }

    operator __UnknownData() const {
        return data;
    }

    operator long() const {
        return data.data.i;
    }
    operator double() const {
        return data.data.f;
    }
    operator char() const {
        return data.data.c;
    }
    operator bool() const {
        return data.data.b;
    }
    operator void* () const {
        return data.data.ptr;
    }

    template<typename T, size_t index = T::index>
    T& get_component() {
        return *((T*)(data.data.ptr + data.virtual_table->tab[index].offset));
    }

    ArrayInfo get_array() {
        return __UnknownData_get_array(data);
    }

    __VirtualTable* virtual_table() const {
        return data.virtual_table;
    }

};

class Reference {

protected:

    void* reference;

public:

    Reference(UnknownData const& data) :
        reference{ __Reference_new_data(data) } {}
    Reference() :
        reference{ __Reference_new_symbol() } {}
    Reference(UnknownData const& parent, __VirtualTable* const virtual_table, void* const property) :
        reference{ __Reference_new_property(parent, virtual_table, property) } {}
    Reference(UnknownData const& array, size_t const i) :
        reference{ __Reference_new_array(array, i) } {}
    Reference(std::initializer_list<Reference> const& list) :
        reference{ __Reference_new_tuple((__Reference_Shared*)std::data(list), list.size()) } {}

    Reference(__Reference_Owned const reference) :
        reference{ reference } {}

    Reference(Reference const& reference) :
        reference{ __Reference_copy(reference) } {}
    Reference(Reference&& reference) :
        reference{ reference.reference } {
        reference.reference = nullptr;
    }

    ~Reference() {
        if (reference != nullptr)
            __Reference_free((__Reference_Owned)reference);
    }

    Reference& operator=(Reference const& reference) {
        if (Reference::reference != reference.reference) {
            __Reference_free((__Reference_Owned)Reference::reference);
            Reference::reference = __Reference_copy((__Reference_Shared)reference.reference);
        }

        return *this;
    }
    Reference& operator=(Reference&& reference) {
        if (Reference::reference != reference.reference) {
            Reference::reference = reference.reference;
            reference.reference = nullptr;
        }

        return *this;
    }

    operator __Reference_Owned() {
        auto tmp = (__Reference_Owned)reference;
        reference = nullptr;
        return tmp;
    }
    operator __Reference_Shared() const {
        return (__Reference_Shared)reference;
    }

    operator __UnknownData() const {
        return get();
    }
    __UnknownData get() const {
        return __Reference_get((__Reference_Shared)reference);
    }

    size_t size() const {
        return __Reference_get_size((__Reference_Shared)reference);
    }

    bool empty() const {
        return size() == 0;
    }

    Reference operator[](size_t i) const {
        return __Reference_get_element((__Reference_Shared)reference, i);
    }

    class iterator {

        Reference& reference;
        size_t i;

    public:

        iterator(Reference& reference, size_t i) :
            reference{ reference }, i{ i } {}

        size_t operator++() {
            return i++;
        }

        size_t operator++(int) {
            return ++i;
        }

        size_t operator--() {
            return i--;
        }

        size_t operator--(int) {
            return --i;
        }

        bool operator==(iterator const& it) const {
            return it.i == i;
        }

        bool operator!=(iterator const& it) const {
            return !(it == *this);
        }

        Reference operator*() const {
            return reference[i];
        }

    };

    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, size());
    }

};

class Function {

public:

    using Filter = bool (*)(Reference const& args);
    using Body = Reference(*)(Reference const& args);

    template<Filter f>
    static bool filter(__Reference_Shared args) {
        Reference r{ (__Reference_Owned)args };
        bool b = f(r);
        r = (Reference)NULL;
        return b;
    }

    template<Body f>
    static __Reference_Owned body(__Reference_Shared args) {
        Reference r{ (__Reference_Owned)args };
        __Reference_Owned o = f(r);
        r = (Reference)NULL;
        return o;
    }

protected:

    __Function_Stack& stack;

public:

    Function(__Function_Stack& function) :
        stack{ function } {}

    operator __Function_Stack() const {
        return stack;
    }

    void push(__FunctionBody body, __FunctionFilter filter = nullptr, std::vector<Reference>&& references = {}) {
        __Function_push(&stack, body, filter, (__Reference_Owned*)references.data(), references.size());
    }

    template<Body body, Filter filter = nullptr>
    void push(std::vector<Reference>&& references = {}) {
        push(Function::body<body>, Function::filter<filter>, references);
    }

    void pop() {
        __Function_pop(&stack);
    }

    Reference operator()(Reference const& args) const {
        return std::move(Reference(__Function_eval(stack, args)));
    }

    void clear() {
        __Function_free(&stack);
    }

};

template<typename ArrayType, typename... Components>
class Type : public Components... {

private:

    template<size_t size>
    struct VirtualTable {
        __VirtualTable_Info info;
        __VirtualTable_TabElement tab[size];
    };

    static constexpr Type<ArrayType, Components...> prototype = Type<ArrayType, Components...>();

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
    static constexpr size_t size = max(Components::index...);

    static constexpr void set_offset(__VirtualTable_TabElement* tab) {}
    template<typename C, typename... Cs>
    static constexpr void set_offset(__VirtualTable_TabElement* tab, C* component, Cs *... components...) {
        tab[C::index].offset = ptrdiff_t(component) - ptrdiff_t(&prototype);
        set_offset(tab, components...);
    }

    static constexpr VirtualTable<size> create_vtable() {
        VirtualTable<size> vtable;
        vtable.info.size = sizeof(Type<ArrayType, Components...>);
        vtable.info.gc_iterator = __GC_iterator;
        if constexpr (std::is_same<ArrayType, void>::value)
            vtable.info.array_vtable = nullptr;
        else
            vtable.info.array_vtable = &ArrayType::vtable;
        set_offset(vtable.tab, static_cast<Components const*>(&prototype)...);
        return vtable;
    }

    static void iterate() {}
    template<typename C, typename... Cs>
    static void iterate(C* component, Cs *... components...) {
        component->iterate();
        iterate(components...);
    }

public:

    static void __GC_iterator(void* object) {
        auto ptr = reinterpret_cast<Type<ArrayType, Components...>*>(object);
        iterate(static_cast<Components*>(ptr)...);
    }

    inline static VirtualTable<size> const vtable = create_vtable();

};

struct Compo {
    static constexpr size_t index = 0;
    void iterate() const {}
};

struct GHDFZGY {
    inline static __VirtualTable vtable;
};

typedef Type<GHDFZGY, Compo> Integer;

void tes() {
    Integer i;
    auto s = Integer::vtable.info.size;
}


#endif
