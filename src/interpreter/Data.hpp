#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <variant>


namespace Interpreter {

    struct Object;

    struct Data : public std::variant<Object*, char, double, long, bool> {

        using std::variant<Object*, char, double, long, bool>::variant;

        template<typename T>
        inline T & get() {
            return std::get<T>(*this);
        }

    };

}


#endif
