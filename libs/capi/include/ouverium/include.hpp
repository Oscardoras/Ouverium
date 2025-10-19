#ifndef __INCLUDE_HPP__
#define __INCLUDE_HPP__

#include <array>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <ouverium/hash_string.h>
#include <ouverium/include.h>
#include <ouverium/types.h>


namespace Ov {

    class ArrayInfo;
    class UnknownData;
    class Reference;
    class Function;


    template<typename T, Ov_VirtualTable* vtable = &T::vtable>
    class Array : protected Ov_Array {

    public:

        Array(size_t capacity = 0, size_t size = 0) :
            Ov_Array{ capacity , size, calloc(capacity, vtable->size) } {}

        operator Ov_ArrayInfo() const {
            return Ov_ArrayInfo{
                .vtable = vtable,
                .array = this
            };
        }

        [[nodiscard]] size_t size() const {
            return size;
        }

        void set_size(size_t const size) {
            return Ov_Array_set_size(*this, size);
        }

        [[nodiscard]] size_t capacity() const {
            return capacity;
        }

        void set_capacity(size_t const size) {
            return Ov_Array_set_capacity(*this, size);
        }

        [[nodiscard]] bool empty() const {
            return size() == 0;
        }

        T& operator[](size_t const i) const {
            return *Ov_Array_get(*this, i);
        }

        class iterator {

            Array<T>& array;
            size_t i;

        public:

            iterator(Array<T>& array, size_t const i) :
                array{ array }, i{ i } {}

            size_t operator++() {
                return ++i;
            }

            size_t operator++(int) {
                return i++;
            }

            size_t operator--() {
                return --i;
            }

            size_t operator--(int) {
                return i--;
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

    class ArrayInfo : public Ov_ArrayInfo {

    public:

        ArrayInfo(Ov_ArrayInfo array) :
            Ov_ArrayInfo{ array } {}

        [[nodiscard]] size_t size() const {
            return array->size;
        }

        void set_size(size_t const size) {
            Ov_Array_set_size(*this, size);
        }

        [[nodiscard]] size_t capacity() const {
            return array->capacity;
        }

        void set_capacity(size_t const size) {
            Ov_Array_set_capacity(*this, size);
        }

        [[nodiscard]] bool empty() const {
            return size() == 0;
        }

        template<typename T>
        [[nodiscard]] T& operator[](size_t const i) const {
            return *static_cast<T const*>(Ov_Array_get(*this, i));
        }

        class iterator {

            ArrayInfo& array;
            size_t i;

        public:

            iterator(ArrayInfo& array, size_t const i) :
                array{ array }, i{ i } {}

            size_t operator++() {
                return ++i;
            }

            size_t operator++(int) {
                return i++;
            }

            size_t operator--() {
                return --i;
            }

            size_t operator--(int) {
                return i--;
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

    class UnknownData : public Ov_UnknownData {

    public:

        UnknownData(Ov_UnknownData const& data) :
            Ov_UnknownData{ data } {}

        UnknownData(Ov_VirtualTable* vtable, union Ov_Data d) :
            Ov_UnknownData{ Ov_UnknownData_from_data(vtable, d) } {}

        UnknownData(Ov_VirtualTable* vtable, void* ptr) :
            Ov_UnknownData{ Ov_UnknownData_from_ptr(vtable, ptr) } {}

        UnknownData(OV_INT i) :
            Ov_UnknownData{ .vtable = &Ov_VirtualTable_Int , .data {.i = i} } {}
        UnknownData(OV_FLOAT f) :
            Ov_UnknownData{ .vtable = &Ov_VirtualTable_Float , .data {.f = f} } {}
        UnknownData(char c) :
            Ov_UnknownData{ .vtable = &Ov_VirtualTable_Char , .data {.c = c} } {}
        UnknownData(bool b) :
            Ov_UnknownData{ .vtable = &Ov_VirtualTable_Bool , .data {.b = b} } {}
        template<class T>
        UnknownData(T* ptr) requires requires() { T::vtable; } :
            Ov_UnknownData{ .vtable = &T::vtable , .data {.ptr = ptr} } {}
        template<typename T>
        UnknownData(T i) requires std::is_integral_v<T> : UnknownData(static_cast<OV_INT>(i)) {}

        friend bool operator==(UnknownData const& a, UnknownData const& b) {
            return Ov_UnknownData_equals(a, b);
        }

        operator Ov_Data() const {
            return Ov_UnknownData::data;
        }

        operator OV_INT() const {
            return data.i;
        }
        operator OV_FLOAT() const {
            return data.f;
        }
        operator char() const {
            return data.c;
        }
        operator bool() const {
            return data.b;
        }
        operator void* () const {
            return data.ptr;
        }
        template<typename T>
        operator T() const requires std::is_integral_v<T> {
            return static_cast<T>(data.i);
        }

        template<typename T>
        T& get_property(const char* name) {
            return *static_cast<T*>(Ov_UnknownData_get_property(*this, hash_string(name)));
        }

        ArrayInfo get_array() {
            return Ov_UnknownData_get_array(*this);
        }

        Function get_function();

    };

    class Reference {

    protected:

        void* reference{};

    public:

        Reference() :
            reference{ Ov_Reference_new_uninitialized() } {}
        Reference(UnknownData const& data) :
            reference{ Ov_Reference_new_data(data) } {}
        static Reference new_symbol(UnknownData const& data) {
            return Ov_Reference_new_symbol(data);
        }
        Reference(UnknownData const& parent, unsigned int const hash) :
            reference{ Ov_Reference_new_property(parent, hash) } {}
        Reference(UnknownData const& array, size_t const i) :
            reference{ Ov_Reference_new_array(array, i) } {}
        Reference(std::initializer_list<Reference> const& list, Ov_VirtualTable* const vtable) :
            reference{ Ov_Reference_new_tuple((Ov_Reference_Shared*) std::data(list), list.size(), vtable) } {}

        Reference(Ov_Reference_Owned reference) :
            reference{ reference } {}
        Reference(Ov_Reference_Shared reference) :
            reference{ Ov_Reference_copy(reference) } {}

        Reference(Reference const& reference) :
            reference{ Ov_Reference_copy(reference) } {}
        Reference(Reference&& reference) noexcept :
            reference{ reference.reference } {
            reference.reference = nullptr;
        }

        ~Reference() {
            if (reference != nullptr)
                Ov_Reference_free(static_cast<Ov_Reference_Owned>(reference));
        }

        Reference& operator=(Reference const& reference) {
            if (Reference::reference != reference.reference) {
                Ov_Reference_free(static_cast<Ov_Reference_Owned>(Reference::reference));
                Reference::reference = Ov_Reference_copy((Ov_Reference_Shared) reference.reference);
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

        operator Ov_Reference_Owned() {
            auto tmp = (Ov_Reference_Owned) reference;
            reference = nullptr;
            return tmp;
        }
        operator Ov_Reference_Shared() const {
            return (Ov_Reference_Shared) reference;
        }

        UnknownData raw() const {
            return Ov_Reference_raw((Ov_Reference_Shared) reference);
        }

        operator UnknownData() const {
            return get();
        }
        UnknownData get() const {
            return Ov_Reference_get((Ov_Reference_Shared) reference);
        }

        size_t size() const {
            return Ov_Reference_get_size((Ov_Reference_Shared) reference);
        }

        bool empty() const {
            return size() == 0;
        }

        Reference operator[](size_t i) const {
            return Ov_Reference_get_element((Ov_Reference_Shared) reference, i);
        }

        class iterator {

            Reference& reference;
            size_t i;

        public:

            iterator(Reference& reference, size_t i) :
                reference{ reference }, i{ i } {}

            size_t operator++() {
                return ++i;
            }

            size_t operator++(int) {
                return i++;
            }

            size_t operator--() {
                return --i;
            }

            size_t operator--(int) {
                return i--;
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
        static inline const std::string parameters = "[" + params + "]";
    };

    template<>
    struct LambdaParameter<Reference> {
        static inline const std::string params = "r";
        static inline const std::string parameters = params;
        static Reference get_arg(Ov_Reference_Shared arg) {
            return arg;
        }
    };

    template<>
    struct LambdaParameter<UnknownData> {
        static inline const std::string params = "r";
        static inline const std::string parameters = params;
        static UnknownData get_arg(Ov_Reference_Shared arg) {
            return Ov_Reference_get(arg);
        }
    };

    template<>
    struct LambdaParameter<std::function<Reference()>> {
        static inline const std::string params = "r()";
        static inline const std::string parameters = params;
        static std::function<Reference()> get_arg(Ov_Reference_Shared arg) {
            return [arg]() -> Reference {
                Reference ref;
                Ov_Expression expr = {
                    .type = Ov_Expression::Ov_EXPRESSION_REFERENCE,
                    .reference = ref
                };
                return Ov_Function_eval(Ov_UnknownData_get_function(Ov_Reference_get(arg)), expr);
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
                if (auto& f = static_cast<Lambda<R(Parameters...)>*>(lambda)->iterate)
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
            static typename P::Type get_arg(Ov_Reference_Shared args[]) {
                return get_arg<P::Type>(args[P::I]);
            }

            template<typename U, size_t... I>
            static U eval(std::function<U(Parameters...)>& f, [[maybe_unused]] Ov_Reference_Shared args[], std::index_sequence<I...> /*unused*/) {
                return f(get_arg<Pair<I>>(args)...);
            }

        public:

            static bool _filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared[]) {
                if (auto& f = static_cast<Lambda<R(Parameters...)>*>(Ov_Reference_get(Ov_Reference_share(captures[0])).data.ptr)->filter)
                    return eval(f, args, std::make_index_sequence<sizeof...(Parameters)>{});
            }

            static Ov_Reference_Owned _body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared[]) {
                if (auto& f = *static_cast<Lambda<R(Parameters...)>*>(Ov_Reference_get(Ov_Reference_share(captures[0])).data.ptr))
                    return eval(f, args, std::make_index_sequence<sizeof...(Parameters)>{});
            }

            static Ov_VirtualTable vtable;
            static inline const std::string parameters = LambdaParameter<Parameters...>::parameters;

            std::function<bool(Parameters...)> filter;

            std::function<void()> iterate;

            using std::function<R(Parameters...)>::function;

        };

    protected:

        Ov_Function& function;

    public:

        Function(Ov_Function& function) :
            function{ function } {}

        operator Ov_Function& () const {
            return function;
        }

        void push(const char* parameters, Ov_FunctionBody body, Ov_FunctionFilter filter = nullptr, size_t local_variables = 0, std::initializer_list<Reference> references = {}) {
            Ov_Function_push(&function, parameters, body, filter, local_variables, (Ov_Reference_Shared*) std::data(references), references.size());
        }

        void push(const char* parameters, Ov_FunctionBody body, Ov_FunctionFilter filter = nullptr, size_t local_variables = 0, std::vector<Reference>&& references = {}) {
            Ov_Function_push(&function, parameters, body, filter, local_variables, (Ov_Reference_Shared*) std::data(references), references.size());
        }

        template<typename R, typename... Args>
        void push(Lambda<R(Args...)> const& lambda) {
            Reference r(UnknownData(&Lambda<R(Args...)>::vtable, Ov_Data{ .ptr = new (Ov_GC_alloc_object(&Lambda<R(Args...)>::vtable)) Lambda<R(Args...)>(lambda) }));

            return Ov_Function_push(&function, Lambda<R(Args...)>::parameters.c_str(), &Lambda<R(Args...)>::_body, &Lambda<R(Args...)>::_filter, 0, (Ov_Reference_Shared*) &r, 1);
        }

        void pop() {
            Ov_Function_pop(&function);
        }

        void clear() {
            Ov_Function_free(&function);
        }

    protected:

        template<typename... T>
        struct Tuple : public std::tuple<T...> {
            using std::tuple<T...>::tuple;
            std::array<Ov_Expression, sizeof...(T)> array;
        };

        Ov_Expression get_expression(Reference const& args) const {
            return Ov_Expression{
                .type = Ov_Expression::Ov_EXPRESSION_REFERENCE,
                .reference = args
            };
        }

        Ov_Expression get_expression(Lambda<Reference()>& lambda) const {
            auto expr = Ov_Expression{
                .type = Ov_Expression::Ov_EXPRESSION_LAMBDA,
                .lambda = NULL
            };
            Function(expr.lambda).push(lambda);
            return expr;
        }

        template<typename... T, size_t... I>
        std::array<Ov_Expression, sizeof...(I)> get_expression(Tuple<T...>& tuple, std::index_sequence<sizeof...(I)> /*unused*/) const {
            return { get_expression(std::get<I>(tuple))... };
        }

        template<typename... T>
        Ov_Expression get_expression(Tuple<T...> const& tuple) const {
            tuple.array = get_expression(tuple, std::make_index_sequence<sizeof...(T)>{});
            return Ov_Expression{
                .type = Ov_Expression::Ov_EXPRESSION_TUPLE,
                .tuple = {
                    .vtable = &Ov_VirtualTable_Object,
                    .size = sizeof...(T),
                    .tab = tuple.array.data()
                }
            };
        }

    public:

        template<typename... Args>
        Reference operator()(Args&&... args) const {
            return Reference(Ov_Function_eval(function, get_expression(Tuple<Args...>{std::forward<Args>(args)...})));
        }

        template<typename Arg>
        Reference operator()(Arg&& arg) const {
            return Reference(Ov_Function_eval(function, get_expression(std::forward<Arg>(arg))));
        }

    };

    template<typename R, typename... Parameters>
    Ov_VirtualTable Function::Lambda<R(Parameters...)>::vtable = {
        .size = sizeof(Function::Lambda<R(Parameters...)>),
        .gc_iterator = Function::Lambda<R(Parameters...)>::iterator,
        .array = {
            .vtable = nullptr,
            .offset = -1
        },
        .function = {
            .offset = 0
        },
        .table_size = 0,
        .table_tab = {}
    };

    inline Function UnknownData::get_function() {
        return *Ov_UnknownData_get_function(*this);
    }

}


#endif
