#include "Interpreter.hpp"


namespace Interpreter {

    std::string Object::to_string() const {
        std::string str;

        for (auto d : array)
            str.push_back(d.get<char>());

        return str;
    }

}
