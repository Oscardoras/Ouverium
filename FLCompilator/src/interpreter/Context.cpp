#include "Function.hpp"
#include "Interpreter.hpp"

#include "../parser/expression/Tuple.hpp"


Object* Context::newObject() {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object());
    return &objects.back();
}

Object* Context::newObject(Object const& object) {
    auto & objects = getGlobal()->objects;
    objects.push_back(object);
    return &objects.back();
}

Object* Context::newObject(bool b) {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(b));
    return &objects.back();
}

Object* Context::newObject(void* ptr) {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(ptr));
    return &objects.back();
}

Object* Context::newObject(long i) {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(i));
    return &objects.back();
}

Object* Context::newObject(double f) {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(f));
    return &objects.back();
}

Object* Context::newObject(char c) {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(c));
    return &objects.back();
}

Object* Context::newObject(size_t tuple_size) {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(tuple_size));
    return &objects.back();
}

Object* Context::newObject(std::string const& str) {
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(str.size()));
    for (long i = 0; i < objects.back().type; i++)
        objects.back().data.a[i+1].o = newObject(str[i]);
    return &objects.back();
}

void dfs(Object* object) {
    object->referenced = true;

    for (auto it = object->fields.begin(); it != object->fields.end(); it++)
        dfs(it->second);
    
    for (auto f : object->functions)
        for (auto it = ((CustomFunction*) f)->externSymbols.begin(); it != ((CustomFunction*) f)->externSymbols.end(); it++)
            if (it->second.type == Reference::Pointer)
                dfs(it->second.pointer);
    
    if (object->type > 0)
        for (long i = 1; i < object->type; i++)
            dfs(object->data.a[i].o);
}

void Context::collect(Object* current) {
    auto context = this;
    while (context != nullptr) {
        for (auto it = symbols.begin(); it != symbols.end(); it++)
            if (it->second.type == Reference::Pointer)
                dfs(it->second.pointer);

        auto parent = context->getParent();
        if (parent != context) context = parent;
        else context = nullptr;
    }

    if (current != nullptr) current->referenced = true;

    auto global = getGlobal();
    for (auto it = global->objects.begin(); it != global->objects.end(); it++)
        if (!it->referenced) {
            auto finalize = it->fields.find("finalize");
            if (finalize != it->fields.end())
                Interpreter::callFunction(*getGlobal(), finalize->second->functions, std::make_shared<Tuple>(), nullptr);

            global->objects.erase(it);
        } else
            it->referenced = false;
    
    if (current != nullptr) current->referenced = false;
}

void Context::addSymbol(std::string const& symbol, Reference const& reference) {
    auto & references = getGlobal()->references;

    if (reference.type == Reference::Pointer) {
        references.push_back(reference.pointer);
        symbols[symbol] = Reference(&references.back());
    } else if (reference.type > 0) {
        references.push_back(newObject((size_t) reference.type));
        for (long i = 0; i < reference.type; i++)
            references.back()->data.a[i+1].o = reference.tuple[i].toObject(*this);
        symbols[symbol] = Reference(&references.back());
    } symbols[symbol] = reference;
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
        return it->second;
    } else if (create) {
        auto & ref = symbols[symbol];
        ref.type = Reference::SymbolReference;
        references.push_back(newObject());
        ref.symbolReference = &references.back();
        return ref;
    } else return Reference((Object**) nullptr);
}

GlobalContext::~GlobalContext() {
    for (auto it = objects.begin(); it != objects.end(); it++) {
        auto finalize = it->fields.find("finalize");
        if (finalize != it->fields.end())
            Interpreter::callFunction(*getGlobal(), finalize->second->functions, std::make_shared<Tuple>(), nullptr);
    }

    for (auto it = cpointers.begin(); it != cpointers.end(); it++) {
        //delete *it;
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

    if (it != symbols.end()) return it->second;
    else {
        auto ref = getGlobal()->getSymbol(symbol, false);
        if (ref.pointer != nullptr) return ref;
        else if (create) {
            auto & references = getGlobal()->references;

            auto & ref = symbols[symbol];
            ref.type = Reference::SymbolReference;
            references.push_back(newObject());
            ref.symbolReference = &references.back();
            return ref;
        } else return Reference((Object**) nullptr);
    }
}