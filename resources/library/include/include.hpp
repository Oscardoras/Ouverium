#ifndef __INCLUDE_HPP__
#define __INCLUDE_HPP__

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "include.h"


class ArrayInfo;
class UnknownData;
class Reference;
class Function;


template<typename T, __VirtualTable* vtable = &T::vtable>
class Array : protected __Array {

public:

    Array(size_t capacity = 0, size_t size = 0) {
        __Array::capacity = capacity;
        __Array::size = size;
        tab = calloc(capacity, vtable->size);
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

    UnknownData(__VirtualTable* vtable, union __Data d) :
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

    template<typename T>
    T& get_property(const char* name) {
        constexpr auto h = hash(name);
        return __UnknownData_get_property(data, h);
    }

    ArrayInfo get_array() {
        return __UnknownData_get_array(data);
    }

    Function get_function();

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

    __Function& function;

public:

    Function(__Function& function) :
        function{ function } {}

    operator __Function& () const {
        return function;
    }

    void push(__FunctionBody body, __FunctionFilter filter = nullptr, std::vector<Reference>&& references = {}) {
        __Function_push(&function, body, filter, (__Reference_Owned*)references.data(), references.size());
    }

    template<Body body, Filter filter = nullptr>
    void push(std::vector<Reference>&& references = {}) {
        push(Function::body<body>, Function::filter<filter>, references);
    }

    void pop() {
        __Function_pop(&function);
    }

    void clear() {
        __Function_free(&function);
    }

private:

    struct Lambda: std::function<Reference()> {
        using std::function<Reference()>::function;
        __FunctionCell cell;
    };

    template<typename... T>
    struct Tuple: public std::tuple<T...> {
        using std::tuple<T...>::tuple;
        std::array<__Expression, sizeof...(T)> array;
    };

    __Expression get_expression(Reference const& args) const {
        return __Expression {
            .type = __Expression::__EXPRESSION_REFERENCE,
            .reference = args
        };
    }

    __Expression get_expression(Lambda & lambda) const {
        lambda.cell = {
            .next = NULL,
            .arguments = NULL,
            .filter = NULL,
            .body = lambda.,

        };
        return __Expression {
            .type = __Expression::__EXPRESSION_LAMBDA,
            .lambda = args
        };
    }

    template<typename... T, size_t... I>
    std::array<__Expression, sizeof...(I)> get_expression(Tuple<T...> & tuple, std::index_sequence<sizeof...(I)>) const {
        return {get_expression(std::get<I>(tuple))...};
    }

    template<typename... T>
    __Expression get_expression(Tuple<T...> const& tuple) const {
        tuple.array = get_expression(tuple, std::make_index_sequence<sizeof...(T)>);
        return __Expression {
            .type = __Expression::__EXPRESSION_TUPLE,
            .tuple.size = sizeof...(T),
            .tuple.tab = tuple.array.data()
        };
    }

public:

    template<typename... T>
    Reference operator()(T... args) const {
        return Reference(__Function_eval(function, get_expression(args)...));
    }

};

Function UnknownData::get_function() {
    return *__UnknownData_get_function(data);
}


#endif
