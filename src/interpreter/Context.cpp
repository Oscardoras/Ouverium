#include <fstream>

#include "Function.hpp"
#include "Interpreter.hpp"


namespace Interpreter {

    Object* Context::new_object(Object const& object) {
        auto& objects = get_global().objects;
        objects.push_back(std::move(object));
        return &objects.back();
    }

    Data& Context::new_reference(Data const& data) {
        auto& references = get_global().references;
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

    void Context::add_symbol(std::string const& symbol, Reference const& reference) {
        symbols.emplace(symbol, reference.to_indirect_reference(*this));
    }

    IndirectReference Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, new_reference()).first->second;
        else
            return it->second;
    }


    GlobalContext::~GlobalContext() {
        for (auto& object : objects)
            object.destruct(*this);
    }

}
