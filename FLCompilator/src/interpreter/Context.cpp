#include "Context.hpp"
#include "InterpreterError.hpp"

#include "../parser/expression/Expression.hpp"


Reference execute(Context & context, std::shared_ptr<Expression> expression);


bool Context::hasSymbol(std::string const& symbol) {
    return symbols.find(symbol) != symbols.end();
}

void Context::addReference(Object* object) {
    if (object->references == 0) {
        for (auto & element : object->fields)
            addReference(element.second);
        if (object->function != nullptr && object->function->type == Function::Custom)
            for (auto & element : ((CustomFunction*) object->function)->objects)
                addReference(element.second);
        if (object->type > 0)
            for (long i = 1; i <= object->type; i++)
                addReference(object->data.a[i].o);
    }
    object->references++;
}

void Context::removeReference(Object* object) {
    if (object->references <= 0) throw "Negative references";

    if (object->references == 1) free(object);
    else object->references--;
}

void Context::finalize(Object * object) {
    auto it = object->fields.find("finalize");
    if (it != object->fields.end()) {
        auto func = it->second;

        if (func->function != nullptr) {
            FunctionContext funcContext(*this);

            if (func->function->type == Function::Custom) {
                for (auto & symbol : ((CustomFunction*) func->function)->objects)
                    funcContext.addSymbol(symbol.first, Reference(symbol.second));

                Object* filter;
                if (((CustomFunction*) func->function)->pointer->filter != nullptr)
                    filter = execute(funcContext, ((CustomFunction*) func->function)->pointer->filter).toObject();
                else filter = new Object(true);

                if (filter->type == Object::Boolean && filter->data.b) {
                    auto result = execute(funcContext, ((CustomFunction*) func->function)->pointer->object);
                    funcContext.freeContext(result);
                    unuse(result);
                    if (filter->references == 0) free(filter);
                } else {
                    funcContext.freeContext();
                    if (filter->references == 0) free(filter);
                    throw InterpreterError();
                }
            } else {
                auto arguments = Reference();
                funcContext.addSymbol("arguments", arguments);

                try {
                    auto result = ((SystemFunction*) func->function)->pointer(arguments, funcContext);
                    funcContext.freeContext(result);
                    unuse(result);
                } catch (InterpreterError & ex) {
                    funcContext.freeContext();
                    throw ex;
                }
            }
        } else throw InterpreterError();
    }
}

void Context::free(Object* object) {
    finalize(object);

    if (object->references == 0) {
        for (auto & element : object->fields)
            if (element.second->references == 0) free(element.second);
        if (object->function != nullptr) {
            if (object->function->type == Function::Custom) {
                for (auto & element : ((CustomFunction*) object->function)->objects)
                    if (element.second->references == 0) free(element.second);
            }
            delete object->function;
        }
        if (object->type > 0) {
            for (long i = 1; i <= object->type; i++)
                if (object->data.a[i].o->references == 0) free(object->data.a[i].o);
            delete[] object->data.a;
        }
    } else {
        for (auto & element : object->fields)
            removeReference(element.second);
        if (object->function != nullptr) {
            if (object->function->type == Function::Custom) {
                for (auto & element : ((CustomFunction*) object->function)->objects)
                    removeReference(element.second);
            }
            delete object->function;
        }
        if (object->type > 0) {
            for (long i = 1; i <= object->type; i++)
                removeReference(object->data.a[i].o);
            std::free(object->data.a);
        }
    }
    delete object;
}

void Context::unuse(Reference & reference) {
    if (reference.isPointerCopy() && reference.ptrCopy->references == 0)
        free(reference.ptrCopy);
    else if (reference.isTuple()) {
        auto n = reference.getTupleSize();
        for (long i = 0; i < n; i++)
            unuse(reference.tuple[i]);
    }
}


GlobalContext::GlobalContext() {
    global = this;
}

Reference GlobalContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);
    if (it != symbols.end()) {
        if (it->second.isPointerCopy())
            return Reference(&it->second.ptrCopy);
        else return it->second;
    } else if (create) {
        auto & ref = symbols[symbol];
        ref.ptrCopy = new Object();
        addReference(ref.ptrCopy);
        return Reference(&ref.ptrCopy);
    } else return Reference((Object**) nullptr);
}

void GlobalContext::addSymbol(std::string const& symbol, Reference const& reference) {
    auto newRef = reference.toSymbolReference();

    auto & ref = symbols[symbol];
    if (ref.isPointerCopy() && ref.ptrCopy != nullptr) removeReference(ref.ptrCopy);
    ref = newRef;
    if (ref.isPointerCopy()) addReference(ref.ptrCopy);
}

GlobalContext::~GlobalContext() {
    for (auto & symbol : symbols)
        if (symbol.second.isPointerCopy())
            removeReference(symbol.second.ptrCopy);
}

FunctionContext::FunctionContext(Context const& parent) {
    global = parent.global;
}

Reference FunctionContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);
    if (it != symbols.end()) {
        if (it->second.isPointerCopy())
            return Reference(&it->second.ptrCopy);
        else return it->second;
    } else {
        auto ref = global->getSymbol(symbol, false);
        if (ref.ptrRef != nullptr) return ref;
        else if (create) {
            auto & ref = symbols[symbol];
            ref.ptrCopy = new Object();
            addReference(ref.ptrCopy);
            return Reference(&ref.ptrCopy);
        } else return Reference((Object**) nullptr);
    }
}

void FunctionContext::addSymbol(std::string const& symbol, Reference const& reference) {
    auto newRef = reference.toSymbolReference();

    auto & ref = symbols[symbol];
    if (ref.isPointerCopy() && ref.ptrCopy != nullptr) removeReference(ref.ptrCopy);
    ref = newRef;
    if (ref.isPointerCopy()) addReference(ref.ptrCopy);
}

void FunctionContext::freeContext() {
    for (auto & symbol : symbols)
        if (symbol.second.isPointerCopy())
            removeReference(symbol.second.ptrCopy);
}

int FunctionContext::checkObject(Object* & object, Reference & result) {
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

void FunctionContext::freeContext(Object* object, Reference & result) {
    if (object->references <= 0) throw "Negative references";

    object->references--;
    if (object->references == 0) {
        finalize(object);

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
            std::free(object->data.a);
        }

        delete object;
    }
}

void FunctionContext::freeContext(Reference & result) {
    for (auto & symbol : symbols)
        if (symbol.second.isPointerCopy()) {
            int r = checkObject(symbol.second.ptrCopy, result);
            if (r == 2) freeContext(symbol.second.ptrCopy, result);
            else if (r == 1) symbol.second.ptrCopy->references--;
        }
}