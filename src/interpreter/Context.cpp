#include <fstream>

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
    long l = str.length();
    auto & objects = getGlobal()->objects;
    objects.push_back(Object(str.size()));
    auto object = &objects.back();
    for (long i = 0; i < l; i++)
        object->data.a[i+1].o = newObject(str[i]);
    return object;
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
    } else symbols[symbol] = reference;
}

bool Context::hasSymbol(std::string const& symbol) const {
    return symbols.find(symbol) != symbols.end();
}

Reference Context::getSymbol(std::string const& symbol) {
    auto & ref = symbols[symbol];

    if (ref.type == Reference::Pointer && ref.pointer == nullptr) {
        auto & references = getGlobal()->references;

        ref.type = Reference::SymbolReference;
        references.push_back(newObject());
        ref.symbolReference = &references.back();
    }

    return ref;
}


GlobalContext* GlobalContext::getGlobal() {
    return this;
}

Context* GlobalContext::getParent() {
    return this;
}

GlobalContext::~GlobalContext() {
    for (auto it = objects.begin(); it != objects.end(); it++) {
        auto finalize = it->fields.find("finalize");
        if (finalize != it->fields.end())
            Interpreter::callFunction(*getGlobal(), finalize->second->functions, std::make_shared<Tuple>(), nullptr);
    }

    for (auto it = cpointers.begin(); it != cpointers.end(); it++)
        delete (std::ios*) *it;
}


FunctionContext::FunctionContext(Context & parent, std::shared_ptr<Position> position) {
    this->parent = &parent;
    this->position = position;
}

GlobalContext* FunctionContext::getGlobal() {
    return parent->getGlobal();
}

Context* FunctionContext::getParent() {
    return parent;
}