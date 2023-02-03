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

        for (auto d : array)
            str.push_back(d.get<char>());
        ~Object();

        return str;
    }

}
