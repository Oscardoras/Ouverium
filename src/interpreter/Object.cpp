#include <exception>
#include <variant>

#include "Interpreter.hpp"


namespace Interpreter {

    Data & Object::get_property(std::string name, Context & context) {
        auto & field = properties[name];
        if (field == Data(nullptr)) field = context.new_object();
        return field;
    }

    std::string Object::to_string() const {
        std::string str;

        for (auto d : array) {
            if (auto c = std::get_if<char>(&d)) str.push_back(*c);
            else throw std::exception();
        }

        return str;
    }

}
