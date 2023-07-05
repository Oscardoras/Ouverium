#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <memory>
#include <variant>


namespace Interpreter {

    class Context;
    struct Function;
    class Object;

    class Data : public std::variant<Object*, char, double, long, bool> {

    public:

        using std::variant<Object*, char, double, long, bool>::variant;

        class BadAccess: public std::exception {};

        template<typename T>
        T & get() {
            return const_cast<T &>(const_cast<Data const&>(*this).get<T>());
        }
        template<typename T>
        T const& get() const {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }

    };

}


#endif
