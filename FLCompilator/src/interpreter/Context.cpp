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
        if (it->second.isCopy())
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
    if (ref.isCopy()) {
        if (ref.ptrCopy != nullptr) ref.ptrCopy->removeReference();
        ref = newRef;
        ref.ptrCopy->addReference();
    } else ref = newRef;
}

GlobalContext::~GlobalContext() {
    for (auto & symbol : symbols)
        if (symbol.second.isCopy())
            symbol.second.ptrCopy->removeReference();
}

FunctionContext::FunctionContext(Context & parent) {
    global = parent.global;
}

Reference FunctionContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);
    if (it != symbols.end()) {
        if (it->second.isCopy())
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
    if (ref.isCopy()) {
        if (ref.ptrCopy != nullptr) ref.ptrCopy->removeReference();
        ref = newRef;
        ref.ptrCopy->addReference();
    } else ref = newRef;
}