#include <fstream>

#include "Function.hpp"
#include "Interpreter.hpp"

#include "../parser/expression/Tuple.hpp"


Object* Context::new_object() {
    auto & objects = get_global()->objects;
    objects.push_back(Object());
    return &objects.back();
}

Object* Context::new_object(Object const& object) {
    auto & objects = get_global()->objects;
    objects.push_back(object);
    return &objects.back();
}

Object* Context::new_object(bool b) {
    auto & objects = get_global()->objects;
    objects.push_back(Object(b));
    return &objects.back();
}

Object* Context::new_object(void* ptr) {
    auto & objects = get_global()->objects;
    objects.push_back(Object(ptr));
    return &objects.back();
}

Object* Context::new_object(long i) {
    auto & objects = get_global()->objects;
    objects.push_back(Object(i));
    return &objects.back();
}

Object* Context::new_object(double f) {
    auto & objects = get_global()->objects;
    objects.push_back(Object(f));
    return &objects.back();
}

Object* Context::new_object(char c) {
    auto & objects = get_global()->objects;
    objects.push_back(Object(c));
    return &objects.back();
}

Object* Context::new_object(size_t tuple_size) {
    auto & objects = get_global()->objects;
    objects.push_back(Object(tuple_size));
    return &objects.back();
}

Object* Context::new_object(std::string const& str) {
    long l = str.length();
    auto & objects = get_global()->objects;
    objects.push_back(Object(str.size()));
    auto object = &objects.back();
    for (long i = 0; i < l; i++)
        object->data.a[i+1].o = new_object(str[i]);
    return object;
}

bool Context::has_symbol(std::string const& symbol) const {
    return symbols.find(symbol) != symbols.end();
}

void Context::add_symbol(std::string const& symbol, Reference const& reference) {
    auto & references = get_global()->references;

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

Reference Context::get_symbol(std::string const& symbol) {
    auto & ref = symbols[symbol];

    if (ref.type == Reference::Pointer && ref.pointer == nullptr) {
        auto & references = get_global()->references;

        ref.type = Reference::SymbolReference;
        references.push_back(new_object());
        ref.symbol_reference = &references.back();
    }

    return ref;
}


GlobalContext* GlobalContext::get_global() {
    return this;
}

Context* GlobalContext::get_parent() {
    return this;
}

GlobalContext::~GlobalContext() {
    for (auto it = objects.begin(); it != objects.end(); it++) {
        auto finalize = it->properties.find("finalize");
        if (finalize != it->properties.end())
            Interpreter::call_function(*get_global(), finalize->second->functions, std::make_shared<Tuple>(), nullptr);
    }

    for (auto it = c_pointers.begin(); it != c_pointers.end(); it++)
        delete (std::ios*) *it;
}


FunctionContext::FunctionContext(Context & parent, std::shared_ptr<Position> position) {
    this->parent = &parent;
    this->position = position;
}

GlobalContext* FunctionContext::get_global() {
    return parent->get_global();
}

Context* FunctionContext::get_parent() {
    return parent;
}
