#include <exception>
#include <variant>

#include "Interpreter.hpp"


namespace Interpreter {

    Data & Object::get_property(std::string name, Context & context) {
        auto it = properties.find(name);
        if (it == properties.end())
            return properties.emplace(name, Data(context.new_object())).first->second;
        else
            return it->second;
    }

    std::string Object::to_string(Context & context) const {
        std::string str;

        for (auto d : array)
            str.push_back(d.get<char>(context));

        return str;
    }

}
