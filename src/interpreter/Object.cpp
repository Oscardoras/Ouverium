#include "Interpreter.hpp"


namespace Interpreter {

    IndirectReference Object::operator[](std::string const& name) {
        return PropertyReference{ *this, name };
    }

    std::string Object::to_string() const {
        std::string str;

        for (auto d : array)
            str.push_back(d.get<char>());

        return str;
    }

    void Object::destruct(Context& context) {
        if (array.size() == 0 && functions.size() == 0 && properties.size() == 0 && !c_obj.has_value())
            return;

        try {
            auto functions = (*this)["destructor"].to_data(context).get<Object*>()->functions;
            if (!functions.empty())
                call_function(context.get_global(), context.get_global().expression, functions, std::make_shared<Parser::Tuple>());
        } catch (std::exception const&) {}
    }

}
