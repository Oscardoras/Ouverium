#include <sstream>

#include "Interpreter.hpp"


namespace Interpreter {

    std::ostream & operator<<(std::ostream & os, Data const& data)  {
        if (auto object = std::get_if<Object*>(&data)) {
            for (auto d : (*object)->array)
                os << d;
        }
        else if (auto c = std::get_if<char>(&data))
            os << *c;
        else if (auto f = std::get_if<double>(&data))
            os << *f;
        else if (auto i = std::get_if<long>(&data))
            os << *i;
        else if (auto b = std::get_if<bool>(&data))
            os << (*b ? "true" : "false");

        return os;
    }

}
