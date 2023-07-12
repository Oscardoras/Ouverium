#include <exception>
#include <variant>

#include "Interpreter.hpp"


namespace Interpreter {

    IndirectReference Object::operator[](std::string name) {
        return PropertyReference{*this, name};
    }

    std::string Object::to_string() const {
        std::string str;

        for (auto d : array)
            str.push_back(d.get<char>());

        return str;
    }

}
