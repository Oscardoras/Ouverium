#include "Interpreter.hpp"


namespace Interpreter {

    std::ostream& operator<<(std::ostream& os, Data const& data) {
        if (auto object = get_if<Object*>(&data)) {
            for (auto d : (*object)->array)
                os << d;
        } else if (auto c = get_if<char>(&data))
            os << *c;
        else if (auto f = get_if<FLOAT>(&data))
            os << *f;
        else if (auto i = get_if<INT>(&data))
            os << *i;
        else if (auto b = get_if<bool>(&data))
            os << (*b ? "true" : "false");

        return os;
    }

}
