#include <fstream>

#include "Function.hpp"
#include "Interpreter.hpp"


namespace Interpreter {

    Object* Context::new_object() {
        auto & objects = static_cast<GlobalContext&>(get_global()).objects;
        objects.push_back(Object());
        return &objects.back();
    }

    Object* Context::new_object(Object && object) {
        auto & objects = static_cast<GlobalContext&>(get_global()).objects;
        objects.push_back(std::move(object));
        return &objects.back();
    }

    Data & Context::new_reference(Data const& data) {
        auto & references = static_cast<GlobalContext&>(get_global()).references;
        references.push_back(data);
        return references.back();
    }

    std::set<std::string> Context::get_symbols() const {
        std::set<std::string> symbols;
        for (auto const& symbol : *this)
            symbols.insert(symbol.first);
        return symbols;
    }

    bool Context::has_symbol(std::string const& symbol) const {
        return symbols.find(symbol) != symbols.end();
    }

    IndirectReference Context::add_symbol(std::string const& symbol, Reference const& reference) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, reference.to_indirect_reference(*this)).first->second;
        else {
            return it->second;
        }
    }

    IndirectReference Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, new_reference(new_object())).first->second;
        else {
            return it->second;
        }
    }


    GlobalContext::~GlobalContext() {
        for (auto const& object : objects) {
            auto it = object.properties.find("destructor");
            if (it != object.properties.end())
                call_function(get_global(), nullptr, it->second.get<Object*>(*this)->functions, std::make_shared<Parser::Tuple>());
        }
    }

}
