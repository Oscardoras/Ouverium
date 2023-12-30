#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <memory>
#include <variant>
#include <ostream>

#include "../Types.hpp"


namespace Interpreter {

    class Context;
    struct Function;
    class Object;

    class Data : public std::variant<Object*, char, FLOAT, INT, bool> {

    public:

        using std::variant<Object*, char, FLOAT, INT, bool>::variant;

        class BadAccess : public std::exception {};

        template<typename T>
        T& get() {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access& e) {
                throw BadAccess();
            }
        }
        template<typename T>
        T const& get() const {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access& e) {
                throw BadAccess();
            }
        }

    };

    std::ostream& operator<<(std::ostream& os, Data const& data);

}


#endif
