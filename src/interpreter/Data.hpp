#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <variant>


namespace Interpreter {

    class Object;

    struct Data : public std::variant<Object*, char, double, long, bool> {

        class BadAccess: public std::exception {};

        using std::variant<Object*, char, double, long, bool>::variant;

        template<typename T>
        inline T & get() {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }

        template<typename T>
        inline T const & get() const {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }
    };

}


#endif
