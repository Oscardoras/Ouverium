#ifndef __INTERPRETER_DATA_HPP__
#define __INTERPRETER_DATA_HPP__

#include <memory>
#include <variant>
#include <ostream>

#include "../Types.hpp"


namespace Interpreter {

    class Object;

    class Data : public std::variant<Object*, char, FLOAT, INT, bool> {

    public:

        using std::variant<Object*, char, FLOAT, INT, bool>::variant;

        template<typename T>
        T& get() {
            return std::get<T>(*this);
        }
        template<typename T>
        T const& get() const {
            return std::get<T>(*this);
        }

    };

    std::ostream& operator<<(std::ostream& os, Data const& data);

}


#endif
