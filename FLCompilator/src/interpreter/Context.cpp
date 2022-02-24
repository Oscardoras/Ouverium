#include "Context.hpp"


bool Context::hasSymbol(std::string const& symbol) {
    return symbols.find(symbol) != symbols.end();
}



GlobalContext::GlobalContext() {
    global = this;
}

Reference GlobalContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);
    if (it != symbols.end()) {
        if (it->second.isPointerCopy())
            return Reference(&it->second.ptrCopy);
        else return Reference(it->second.ptrRef);
    } else if (create) {
        auto & ref = symbols[symbol];
        ref.ptrCopy = new Object();
        ref.ptrCopy->addReference();
        return Reference(&ref.ptrCopy);
    } else return Reference((Object**) nullptr);
}

void GlobalContext::addSymbol(std::string const& symbol, Reference const& reference) {
    auto newRef = reference.toSymbolReference();

    auto & ref = symbols[symbol];
    if (ref.isPointerCopy()) {
        if (ref.ptrCopy != nullptr) ref.ptrCopy->removeReference();
        ref = newRef;
        ref.ptrCopy->addReference();
    } else ref = newRef;
}

GlobalContext::~GlobalContext() {
    for (auto & symbol : symbols)
        if (symbol.second.isPointerCopy())
            symbol.second.ptrCopy->removeReference();
}

FunctionContext::FunctionContext(Context & parent) {
    global = parent.global;
}

Reference FunctionContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);
    if (it != symbols.end()) {
        if (it->second.isPointerCopy())
            return Reference(&it->second.ptrCopy);
        else return Reference(it->second.ptrRef);
    } else {
        auto ref = global->getSymbol(symbol, false);
        if (ref.ptrRef != nullptr) return ref;
        else if (create) {
            auto & ref = symbols[symbol];
            ref.ptrCopy = new Object();
            ref.ptrCopy->addReference();
            return Reference(&ref.ptrCopy);
        } else return Reference((Object**) nullptr);
    }
}

void FunctionContext::addSymbol(std::string const& symbol, Reference const& reference) {
    auto newRef = reference.toSymbolReference();

    auto & ref = symbols[symbol];
    if (ref.isPointerCopy()) {
        if (ref.ptrCopy != nullptr) ref.ptrCopy->removeReference();
        ref = newRef;
        ref.ptrCopy->addReference();
    } else ref = newRef;
}

void FunctionContext::free() {
    for (auto & symbol : symbols)
        if (symbol.second.isPointerCopy())
            symbol.second.ptrCopy->removeReference();
}

int checkObject(Object* & object, Reference & result) {
    if (result.isPointerReference()) {
        if (result.ptrRef == &object) {
            result.type = Reference::PointerCopy;
            result.ptrCopy = object;
            return 1;
        } else return 2;
    } else if (result.isArrayReference()) {
        if (result.getArrayReference() == &object) {
            result.type = Reference::PointerCopy;
            result.ptrCopy = object;
            return 1;
        } else return 2;
    } else if (result.isPointerCopy()) {
        return result.ptrCopy == object && object->references == 1 ? 1 : 2;
    } else {
        int r = 2;
        for (auto i = 0; i < result.type; i++) {
            int t = checkObject(object, result.tuple[i]);
            if (t == 0 && r > 0) r = 0;
            else if (t == 1 && r > 1) r = 1;
        }
        return r;
    }
}

void freeContext(Object* object, Reference & result) {
    if (object->references <= 0) throw "Negative references";

    object->references--;
    if (object->references == 0) {
        for (auto & element : object->fields) {
            int r = checkObject(element.second, result);
            if (r == 2) freeContext(element.second, result);
            else if (r == 1) element.second->references--;
        }
        if (object->function != nullptr) {
            if (object->function->type == Function::Custom)
                for (auto & element : ((CustomFunction*) object->function)->objects) {
                    int r = checkObject(element.second, result);
                    if (r == 2) freeContext(element.second, result);
                    else if (r == 1) element.second->references--;
                }
            delete object->function;
        }

        if (object->type > 0) {
            for (long i = 1; i <= object->type; i++) {
                int r = checkObject(object->data.a[i].o, result);
                if (r == 2) freeContext(object->data.a[i].o, result);
                else if (r == 1) object->data.a[i].o->references--;
            }
            delete[] object->data.a;
        }

        operator delete(object);
    }
}

void FunctionContext::free(Reference & result) {
    for (auto & symbol : symbols)
        if (symbol.second.isPointerCopy()) {
            int r = checkObject(symbol.second.ptrCopy, result);
            if (r == 2) freeContext(symbol.second.ptrCopy, result);
            else if (r == 1) symbol.second.ptrCopy->references--;
        }
}