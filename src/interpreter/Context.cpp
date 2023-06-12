#include <fstream>

#include "Function.hpp"
#include "Interpreter.hpp"


namespace Interpreter {

    Object* Context::new_object() {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        return &objects.back();
    }

    Object* Context::new_object(Object && object) {
        auto & objects = get_global().objects;
        objects.push_back(std::move(object));
        return &objects.back();
    }

    Object* Context::new_object(std::string const& str) {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        auto object = &objects.back();
        for (auto c : str)
            object->array.push_back(c);
        return object;
    }

    Data & Context::new_reference(Data const& data) {
        auto & references = get_global().references;
        references.push_back(data);
        return references.back();
    }

    bool Context::has_symbol(std::string const& symbol) const {
        return symbols.find(symbol) != symbols.end();
    }

    Data & Context::add_symbol(std::string const& symbol, Reference const& reference) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, reference.to_symbol_reference(*this)).first->second;
        else {
            return it->second;
        }
    }

    Data & Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, new_reference(new_object())).first->second;
        else {
            return it->second;
        }
    }


    GlobalContext & GlobalContext::get_global() {
        return *this;
    }

    Context & GlobalContext::get_parent() {
        return *this;
    }

    GlobalContext::~GlobalContext() {
        for (auto it = objects.begin(); it != objects.end(); it++) {
            auto finalize = it->properties.find("finalize");
            if (finalize != it->properties.end())
                call_function(get_global(), nullptr, finalize->second.get<Object*>(*this)->functions, std::make_shared<Parser::Tuple>());
        }
    }


    GlobalContext & FunctionContext::get_global() {
        return parent.get().get_global();
    }

    Context & FunctionContext::get_parent() {
        return parent;
    }

}
