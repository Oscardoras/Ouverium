#include <fstream>

#include "Function.hpp"
#include "Interpreter.hpp"


namespace Interpreter {

    Object* Context::new_object() {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        return &objects.back();
    }

    Object* Context::new_object(Object const& object) {
        auto & objects = get_global().objects;
        objects.push_back(object);
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

    Data & Context::new_reference(Data data) {
        auto & references = get_global().references;
        references.push_back(data);
        return references.back();
    }

    bool Context::has_symbol(std::string const& symbol) const {
        return symbols.find(symbol) != symbols.end();
    }

/*
    void Context::add_symbol(std::string const& symbol, Reference const& reference) {
        auto & references = get_global().references;

        if (reference.type == Reference::Pointer) {
            references.push_back(reference.pointer);
            symbols[symbol] = Reference(&references.back());
        } else if (reference.type > 0) {
            references.push_back(new_object((size_t) reference.type));
            for (long i = 0; i < reference.type; i++)
                references.back()->data.a[i+1].o = reference.tuple[i].to_object(*this);
            symbols[symbol] = Reference(&references.back());
        } else symbols[symbol] = reference;
    }
*/

    Data & Context::add_symbol(std::string const& symbol, Reference const& reference) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            symbols.emplace(symbol, reference.to_symbol_reference(*this));

        return symbols.at(symbol);
    }

    Data & Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            symbols.emplace(symbol, new_reference(new_object()));

        return symbols.at(symbol);
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
                if (auto object = std::get_if<Object*>(&finalize->second))
                    call_function(get_global(), nullptr, (*object)->functions, std::make_shared<Tuple>());
        }

        for (auto it = c_pointers.begin(); it != c_pointers.end(); it++)
            delete (std::ios*) *it;
    }


    GlobalContext & FunctionContext::get_global() {
        return parent.get().get_global();
    }

    Context & FunctionContext::get_parent() {
        return parent;
    }

}
