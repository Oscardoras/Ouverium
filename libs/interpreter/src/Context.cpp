#include <memory>
#include <set>
#include <string>
#include <utility>

#include "system_functions/SystemFunction.hpp"

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Interpreter {

    Context::Context(std::shared_ptr<Parser::Expression> caller) :
        caller(std::move(caller)) {}

    std::set<std::string> Context::get_symbols() const {
        std::set<std::string> symbols;
        for (auto const& symbol : *this)
            symbols.insert(symbol.first);
        return symbols;
    }

    bool Context::has_symbol(std::string const& symbol) const {
        return symbols.find(symbol) != symbols.end();
    }

    void Context::add_symbol(std::string const& symbol, IndirectReference const& indirect_reference) {
        symbols.emplace(symbol, indirect_reference);
    }

    void Context::add_symbol(std::string const& symbol, Data const& data) {
        symbols.emplace(symbol, GC::new_reference(data));
    }

    IndirectReference Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, GC::new_reference()).first->second;
        else
            return it->second;
    }


    GlobalContext::GlobalContext(std::shared_ptr<Parser::Expression> expression) :
        Context(std::move(expression)), system(GC::new_object()) {
        SystemFunctions::init(*this);
    }

    FunctionContext::FunctionContext(Context& parent, std::shared_ptr<Parser::Expression> caller) :
        Context(std::move(caller)), parent(parent), recursion_level(parent.get_recurion_level() + 1) {}

}
