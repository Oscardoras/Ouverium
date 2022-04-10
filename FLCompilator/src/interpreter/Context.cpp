#include "Context.hpp"
#include "Interpreter.hpp"
#include "InterpreterError.hpp"

#include "../parser/expression/Tuple.hpp"


Object* Context::addObject(Object* object) {
    getGlobal()->objects.push_back(object);
    return object;
}

void dfs(Object* object) {
    object->referenced = true;

    for (auto it = object->fields.begin(); it != object->fields.end(); it++)
        dfs(it->second);
    
    for (auto f : object->functions)
        for (auto it = ((CustomFunction*) f)->objects.begin(); it != ((CustomFunction*) f)->objects.end(); it++)
            dfs(it->second);
    
    if (object->type > 0)
        for (long i = 1; i < object->type; i++)
            dfs(object->data.a[i].o);
}

void Context::collect(Object* current) {
    auto context = this;
    while (context != nullptr) {
        for (auto it = symbols.begin(); it != symbols.end(); it++)
            if (it->second.isPointerCopy())
                dfs(it->second.ptrCopy);

        auto parent = context->getParent();
        if (parent != context) context = parent;
        else context = nullptr;
    }

    if (current != nullptr) current->referenced = true;

    auto global = getGlobal();
    for (auto it = global->objects.begin(); it != global->objects.end(); it++)
        if (!(*it)->referenced) {
            auto finalize = (*it)->fields.find("finalize");
            if (finalize != (*it)->fields.end())
                Interpreter::callFunction(*getGlobal(), finalize->second->functions, std::make_shared<Tuple>());

            delete *it;
            global->objects.erase(it);
        } else
            (*it)->referenced = false;
    
    if (current != nullptr) current->referenced = false;
}

void Context::addSymbol(std::string const& symbol, Reference const& reference) {
    symbols[symbol] = reference.toSymbolReference(*this);
}

bool Context::hasSymbol(std::string const& symbol) const {
    return symbols.find(symbol) != symbols.end();
}

GlobalContext* GlobalContext::getGlobal() {
    return this;
}

Context* GlobalContext::getParent() {
    return this;
}

Reference GlobalContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);
    if (it != symbols.end()) {
        if (it->second.isPointerCopy())
            return Reference(&it->second.ptrCopy);
        else return it->second;
    } else if (create) {
        auto & ref = symbols[symbol];
        ref.ptrCopy = addObject(new Object());
        return Reference(&ref.ptrCopy);
    } else return Reference((Object**) nullptr);
}

GlobalContext::~GlobalContext() {
    for (auto it = objects.begin(); it != objects.end(); it++) {
        auto finalize = (*it)->fields.find("finalize");
        if (finalize != (*it)->fields.end())
            Interpreter::callFunction(*getGlobal(), finalize->second->functions, std::make_shared<Tuple>());

        delete *it;
    }
}


FunctionContext::FunctionContext(Context & parent) {
    this->parent = &parent;
}

GlobalContext* FunctionContext::getGlobal() {
    return parent->getGlobal();
}

Context* FunctionContext::getParent() {
    return parent;
}

Reference FunctionContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);

    if (it != symbols.end()) {
        if (it->second.isPointerCopy())
            return Reference(&it->second.ptrCopy);
        else return it->second;
    } else {
        auto ref = getGlobal()->getSymbol(symbol, false);
        if (ref.ptrRef != nullptr) return ref;
        else if (create) {
            auto & ref = symbols[symbol];
            ref.ptrCopy = addObject(new Object());
            return Reference(&ref.ptrCopy);
        } else return Reference((Object**) nullptr);
    }
}