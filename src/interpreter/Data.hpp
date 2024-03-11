#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <exception>
#include <ostream>
#include <variant>

#include "../Types.hpp"


namespace Interpreter {

    struct Object;

    class Data : protected std::variant<Object*, char, OV_FLOAT, OV_INT, bool> {

    public:

        using std::variant<Object*, char, OV_FLOAT, OV_INT, bool>::variant;

        using std::variant<Object*, char, OV_FLOAT, OV_INT, bool>::operator=;
        friend auto operator==(Data const& a, Data const& b) {
            return static_cast<std::variant<Object*, char, OV_FLOAT, OV_INT, bool> const&>(a) == static_cast<std::variant<Object*, char, OV_FLOAT, OV_INT, bool> const&>(b);
        }

        class BadAccess : public std::exception {};

        template<typename T>
        T const& get() const {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access const&) {
                throw BadAccess();
            }
        }
        template<typename T>
        friend T const* get_if(Data const* data) {
            return std::get_if<T>(data);
        }
        template<typename T>
        bool is() const {
            return std::holds_alternative<T>(*this);
        }

    };

}


#endif
