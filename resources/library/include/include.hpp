#ifndef __INCLUDE_HPP__
#define __INCLUDE_HPP__

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "hash_string.h"
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
        return *static_cast<T*>(__UnknownData_get_property(data, h));
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
    Reference(__Reference_Shared const reference) :
        reference{ __Reference_copy(reference) } {}

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

    operator UnknownData() const {
        return get();
    }
    UnknownData get() const {
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

template<typename... Parameters>
struct LambdaParameter {
    static inline const std::string params = "";
    static inline const std::string parameters = "";
};

template<typename Parameter0, typename Parameter1, typename... Parameters>
struct LambdaParameter<Parameter0, Parameter1, Parameters...> {
    static inline const std::string params = LambdaParameter<Parameter0>::params + "," + LambdaParameter<Parameter1, Parameters...>::params;
    static inline const std::string parameters = "(" + params + ")";
};

template<>
struct LambdaParameter<Reference> {
    static inline const std::string params = "r";
    static inline const std::string parameters = params;
    static Reference get_arg(__Reference_Shared arg) {
        return arg;
    }
};

template<>
struct LambdaParameter<UnknownData> {
    static inline const std::string params = "r";
    static inline const std::string parameters = params;
    static UnknownData get_arg(__Reference_Shared arg) {
        return __Reference_get(arg);
    }
};

template<>
struct LambdaParameter<std::function<Reference()>> {
    static inline const std::string params = "r()";
    static inline const std::string parameters = params;
    static std::function<Reference()> get_arg(__Reference_Shared arg) {
        return [arg]() -> Reference {
            __Expression expr = {
                .type = __Expression::__EXPRESSION_TUPLE,
                .tuple = {
                    .size = 0,
                    .tab = nullptr
                }
            };
            return __Function_eval(*__UnknownData_get_function(__Reference_get(arg)), expr);
        };
    }
};

class Function {

public:

    template<typename R, typename... Parameters>
    class Lambda;

    template<typename R, typename... Parameters>
    class Lambda<R(Parameters...)> : public std::function<R(Parameters...)> {

    protected:

        static void iterator(void* lambda) {
            if (auto & f = static_cast<Lambda<R(Parameters...)>*>(lambda)->iterate)
                f();
        }

        static void destructor(void* lambda) {
            static_cast<Lambda<R(Parameters...)>*>(lambda)->~Lambda();
        }

        template<size_t I>
        struct Pair {
            using Type = decltype(std::get<I>(std::tuple<Parameters...> {}));
            static constexpr size_t Index = I;
        };

        template<typename P>
        static typename P::Type get_arg(__Reference_Shared args[]) {
            return get_arg<P::Type>(args[P::I]);
        }

        template<typename U, size_t... I>
        static U eval(std::function<U(Parameters...)> & f, [[maybe_unused]] __Reference_Shared args[], std::index_sequence<I...>) {
            return f(get_arg<Pair<I>>(args)...);
        }

    public:

        static bool _filter(__Reference_Owned capture[], __Reference_Shared args[]) {
            if (auto & f = static_cast<Lambda<R(Parameters...)>*>(__Reference_get(__Reference_share(capture[0])).data.ptr)->filter)
                return eval(f, args, std::make_index_sequence<sizeof...(Parameters)>{});
        }

        static __Reference_Owned _body(__Reference_Owned capture[], __Reference_Shared args[]) {
            if (auto & f = *static_cast<Lambda<R(Parameters...)>*>(__Reference_get(__Reference_share(capture[0])).data.ptr))
                return eval(f, args, std::make_index_sequence<sizeof...(Parameters)>{});
        }

        static __VirtualTable vtable;
        static inline const std::string parameters = LambdaParameter<Parameters...>::parameters;

        std::function<bool(Parameters...)> filter;

        std::function<void()> iterate;

        using std::function<R(Parameters...)>::function;

    };

protected:

    __Function& function;

public:

    Function(__Function& function) :
        function{ function } {}

    operator __Function& () const {
        return function;
    }

    void push(const char* parameters, __FunctionBody body, __FunctionFilter filter = nullptr, std::initializer_list<Reference> references = {}) {
        __Function_push(&function, parameters, body, filter, (__Reference_Owned*)std::data(references), references.size());
    }

    void push(const char* parameters, __FunctionBody body, __FunctionFilter filter = nullptr, std::vector<Reference>&& references = {}) {
        __Function_push(&function, parameters, body, filter, (__Reference_Owned*)std::data(references), references.size());
    }

    template<typename R, typename... Args>
    void push(Lambda<R(Args...)> const& lambda) {
        auto cell = static_cast<__FunctionCell*>(malloc(sizeof(__FunctionCell) + sizeof(__Reference_Owned)));

        cell->next = function;
        cell->parameters = Lambda<R(Args...)>::parameters.c_str();
        cell->filter = &Lambda<R(Args...)>::_filter;
        cell->body = &Lambda<R(Args...)>::_body;
        cell->captures.size = 1;
        cell->captures.tab[0] = Reference(UnknownData(&Lambda<R(Args...)>::vtable, __Data{ .ptr = new (__GC_alloc_object(&Lambda<R(Args...)>::vtable)) Lambda<R(Args...)>(lambda) }));

        function = cell;
    }

    void pop() {
        __Function_pop(&function);
    }

    void clear() {
        __Function_free(&function);
    }

protected:

    template<typename... T>
    struct Tuple : public std::tuple<T...> {
        using std::tuple<T...>::tuple;
        std::array<__Expression, sizeof...(T)> array;
    };

    __Expression get_expression(Reference const& args) const {
        return __Expression{
            .type = __Expression::__EXPRESSION_REFERENCE,
            .reference = args
        };
    }

    __Expression get_expression(Lambda<Reference()>& lambda) const {
        auto expr = __Expression{
            .type = __Expression::__EXPRESSION_LAMBDA,
            .lambda = NULL
        };
        Function(expr.lambda).push(lambda);
        return expr;
    }

    template<typename... T, size_t... I>
    std::array<__Expression, sizeof...(I)> get_expression(Tuple<T...>& tuple, std::index_sequence<sizeof...(I)>) const {
        return { get_expression(std::get<I>(tuple))... };
    }

    template<typename... T>
    __Expression get_expression(Tuple<T...> const& tuple) const {
        tuple.array = get_expression(tuple, std::make_index_sequence<sizeof...(T)>{});
        return __Expression{
            .type = __Expression::__EXPRESSION_TUPLE,
            .tuple = {
                .size = sizeof...(T),
                .tab = tuple.array.data()
            }
        };
    }

public:

    template<typename... Args>
    Reference operator()(Args&&... args) const {
        return Reference(__Function_eval(function, get_expression(Tuple<Args...>{args...})));
    }

    template<typename Arg>
    Reference operator()(Arg&& arg) const {
        return Reference(__Function_eval(function, get_expression(arg)));
    }

};

template<typename R, typename... Parameters>
__VirtualTable Function::Lambda<R(Parameters...)>::vtable = {
    .size = sizeof(Function::Lambda<R(Parameters...)>),
    .gc_iterator = Function::Lambda<R(Parameters...)>::iterator,
    .gc_destructor = Function::Lambda<R(Parameters...)>::destructor,
    .array = {
        .vtable = nullptr,
        .offset = 0
    },
    .function = {
        .offset = 0
    },
    .table = {
        .size = 0,
        .tab = {}
    }
};

Function UnknownData::get_function() {
    return *__UnknownData_get_function(data);
}


#endif
