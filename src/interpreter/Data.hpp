#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

// IWYU pragma: private; include "Interpreter.hpp"

#include <any>
#include <cstddef>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>

#include <ouverium/types.h>


namespace Interpreter {

    struct Object;
    struct PropertyReference;
    struct ArrayReference;

    using ObjectPtr = std::shared_ptr<Object>;

    class Data : protected std::any {

    public:

        using Comparators = std::map<std::pair<std::type_index, std::type_index>, std::function<bool(std::any const&, std::any const&)>>;
        using Comparator = std::pair<std::pair<std::type_index, std::type_index>, std::function<bool(std::any const&, std::any const&)>>;

        static Comparators comparators;

        template<typename T, typename U>
        static inline Comparator SimpleComparator = {
            {std::type_index(typeid(T)), std::type_index(typeid(U))},
            [](std::any const& a, std::any const& b) {
                return std::any_cast<T>(a) == std::any_cast<U>(b);
            }
        };


        Data() = default;
        explicit Data(ObjectPtr object) : std::any(object) {}
        explicit Data(char c) : std::any(c) {}
        explicit Data(OV_FLOAT f) : std::any(f) {}
        explicit Data(OV_INT i) : std::any(i) {}
        explicit Data(bool b) : std::any(b) {}
        explicit Data(std::any const& any) : std::any(any) {}

        Data& operator=(ObjectPtr object) {
            static_cast<std::any&>(*this) = object;
            return *this;
        }
        Data& operator=(char c) {
            static_cast<std::any&>(*this) = c;
            return *this;
        }
        Data& operator=(OV_FLOAT f) {
            static_cast<std::any&>(*this) = f;
            return *this;
        }
        Data& operator=(OV_INT i) {
            static_cast<std::any&>(*this) = i;
            return *this;
        }
        Data& operator=(bool b) {
            static_cast<std::any&>(*this) = b;
            return *this;
        }
        Data& operator=(std::any const& any) {
            static_cast<std::any&>(*this) = any;
            return *this;
        }

        friend bool operator==(Data const& a, Data const& b);

        class BadAccess : public std::exception {};

        template<typename T>
        [[nodiscard]] T const& get() const {
            try {
                return std::any_cast<T const&>(*this);
            } catch (std::bad_any_cast const&) {
                throw BadAccess();
            }
        }
        template<typename T>
        [[nodiscard]] friend T const* get_if(Data const* data) {
            return std::any_cast<T const>(data);
        }
        template<typename T>
        [[nodiscard]] bool is() const {
            return std::any_cast<T const>(this);
        }

        [[nodiscard]] PropertyReference get_property(std::string const& name);

        [[nodiscard]] ArrayReference get_at(size_t index);

    };

}


#endif
