#include "Interpreter.hpp"


namespace Interpreter {

    ObjectPtr Context::new_object(Object const& object) {
        auto& global = get_global();
        auto& objects = global.objects;
        if (objects.size() >= 2 * global.last_size) {
            global.GC_collect();
        }
        std::lock_guard guard(global.mutex);
        objects.push_back({ std::move(object), false });
        return --objects.end();
    }

    Data& Context::new_reference(Data const& data) {
        auto& global = get_global();
        auto& references = global.references;
        std::lock_guard guard(global.mutex);
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

    void Context::add_symbol(std::string const& symbol, IndirectReference const& indirect_reference) {
        symbols.emplace(symbol, indirect_reference);
    }

    void Context::add_symbol(std::string const& symbol, Data const& data) {
        symbols.emplace(symbol, new_reference(data));
    }

    IndirectReference Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, new_reference()).first->second;
        else
            return it->second;
    }


    GlobalContext::GlobalContext(std::shared_ptr<Parser::Expression> expression) :
        Context(expression), system{ new_object() } {
        contexts.insert(this);
        SystemFunctions::init(*this);
    }

    FunctionContext::FunctionContext(Context& parent, std::shared_ptr<Parser::Expression> caller) :
        Context(caller), parent(parent), recursion_level(parent.get_recurion_level() + 1) {
        auto& global = get_global();
        std::lock_guard guard(global.mutex);
        global.contexts.insert(this);
    }

    FunctionContext::~FunctionContext() {
        auto& global = get_global();
        std::lock_guard guard(global.mutex);
        global.contexts.erase(this);
    }

}
