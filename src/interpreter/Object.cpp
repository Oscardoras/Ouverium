#include "Interpreter.hpp"


namespace Interpreter {

    std::string Object::to_string() const {
        std::string str;

        for (auto d : array)
            str.push_back(d.get<char>());

        return str;
    }

    void Object::destruct(Context& context) {
        if (weak_ref) {
            weak_ref->c_obj.get<WeakReference>().obj = nullptr;
        }

        if (array.size() == 0 && functions.size() == 0 && properties.size() == 0 && !c_obj.has_value())
            return;

        try {
            if (this->properties.contains("destructor"))
                call_function(context.get_global(), nullptr, PropertyReference{ Data(this), "destructor" }, std::make_shared<Parser::Tuple>());
        } catch (Interpreter::Exception const&) {}
    }

}
