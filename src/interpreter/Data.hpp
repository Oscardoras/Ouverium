#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <memory>
#include <variant>
#include <ostream>


namespace Interpreter {

    class Context;
    struct Function;
    class Object;

    class Data : public std::variant<Object*, char, double, long, bool> {

    public:

        using std::variant<Object*, char, double, long, bool>::variant;

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
