#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <any>
#include <exception>
#include <ostream>
#include <typeindex>

#include "../Types.hpp"


namespace Interpreter {

    struct Object;

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
        explicit Data(Object* object) : std::any(object) {}
        explicit Data(char c) : std::any(c) {}
        explicit Data(OV_FLOAT f) : std::any(f) {}
        explicit Data(OV_INT i) : std::any(i) {}
        explicit Data(bool b) : std::any(b) {}
        explicit Data(std::any const& any) : std::any(any) {}

        Data& operator=(Object* object) {
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

        friend bool operator==(Data const& a, Data const& b) {
            if (!static_cast<std::any const&>(a).has_value() && !static_cast<std::any const&>(b).has_value())
                return true;

            auto it = Data::comparators.find({ std::type_index(static_cast<std::any const&>(a).type()), std::type_index(static_cast<std::any const&>(b).type()) });
            if (it != Data::comparators.end()) {
                return it->second(a, b);
            } else
                return false;
        }
        friend bool operator!=(Data const& a, Data const& b) {
            return !(a == b);
        }

        class BadAccess : public std::exception {};

        template<typename T>
        T const& get() const {
            try {
                return std::any_cast<T const&>(*this);
            } catch (std::bad_any_cast const&) {
                throw BadAccess();
            }
        }
        template<typename T>
        friend T const* get_if(Data const* data) {
            return std::any_cast<T const>(data);
        }
        template<typename T>
        bool is() const {
            return std::any_cast<T const>(this);
        }

    };

}


#endif
