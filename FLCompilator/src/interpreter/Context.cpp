#include "Context.hpp"
#include "Interpreter.hpp"
#include "InterpreterError.hpp"

#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"


Object* Context::addObject(Object* object) {
    getGlobal()->objects.push_back(object);
    return object;
}

void dfs(Object* object) {
    object->referenced = true;

    for (auto it = object->fields.begin(); it != object->fields.end(); it++)
        dfs(it->second);
    
    if (object->function != nullptr && object->function->type == Function::Custom)
        for (auto it = ((CustomFunction*) object->function)->objects.begin(); it != ((CustomFunction*) object->function)->objects.end(); it++)
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
                if (finalize->second->function != nullptr)
                    Interpreter::callFunction(*getGlobal(), finalize->second->function, std::make_shared<Tuple>());

            delete *it;
            global->objects.erase(it);
        } else
            (*it)->referenced = false;
    
    if (current != nullptr) current->referenced = false;
}

void Context::addSymbol(std::string const& symbol, Reference const& reference) {
    symbols[symbol] = reference.toSymbolReference();
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
    collect(nullptr);
}


FunctionContext::FunctionContext(Context const& parent, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
    this->parameters = parameters;
    this->arguments = arguments;
}

GlobalContext* FunctionContext::getGlobal() {
    return parent->getGlobal();
}

Context* FunctionContext::getParent() {
    return parent;
}

void setReferences(FunctionContext & function_context, std::shared_ptr<Expression> expression, Reference reference) {
    if (expression->type == Expression::Symbol) {
        auto symbol = std::static_pointer_cast<Symbol>(expression);

        function_context.addSymbol(symbol->name, reference);
    } else if (expression->type == Expression::Tuple) {
        auto tuple = std::static_pointer_cast<Tuple>(expression);

        if (reference.isTuple()) {
            if (reference.getTupleSize() == tuple->objects.size()) {
                for (size_t i = 0; i < tuple->objects.size(); i++)
                    setReferences(function_context, tuple->objects[i], reference.tuple[i]);
            } else throw InterpreterError();
        } else {
            auto object = reference.toObject();
            if (object->type == tuple->objects.size()) {
                for (unsigned long i = 1; i <= tuple->objects.size(); i++)
                    setReferences(function_context, tuple->objects[i], Reference(&object->data.a[i].o));
            } else throw InterpreterError();
        }
    }
}

enum FindStatus {
    NotFound,
    Found,
    Set
};

FindStatus findSymbol(Context & context, FunctionContext & function_context, std::string symbol, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
    if (arguments != nullptr) {
        if (parameters->type == Expression::Symbol) {
            if (std::static_pointer_cast<Symbol>(parameters)->name == symbol) {
                setReferences(function_context, parameters, Interpreter::execute(context, arguments));
                return Set;
            } else
                return NotFound;
        } else if (parameters->type == Expression::Tuple) {
            auto p_tuple = std::static_pointer_cast<Tuple>(parameters);

            if (arguments->type == Expression::Tuple) {
                auto a_tuple = std::static_pointer_cast<Tuple>(arguments);

                if (a_tuple->objects.size() == p_tuple->objects.size())
                    for (size_t i = 0; i < p_tuple->objects.size(); i++)
                        if (findSymbol(context, function_context, symbol, p_tuple->objects[i], a_tuple->objects[i]) != NotFound)
                            return Set;
            } else {
                for (size_t i = 0; i < p_tuple->objects.size(); i++)
                    if (findSymbol(context, function_context, symbol, p_tuple->objects[i], nullptr) == Found) {
                        setReferences(function_context, parameters, Interpreter::execute(context, arguments));
                        return Set;
                    }
            }
        }
    } else {
        if (parameters->type == Expression::Symbol) {
            if (std::static_pointer_cast<Symbol>(parameters)->name == symbol) return Found;
            else return NotFound;
        } else if (parameters->type == Expression::Tuple) {
            auto p_tuple = std::static_pointer_cast<Tuple>(parameters);

            for (size_t i = 0; i < p_tuple->objects.size(); i++)
                return findSymbol(context, function_context, symbol, p_tuple->objects[i], nullptr);
        }
    }
}

Reference FunctionContext::getSymbol(std::string const& symbol, bool const& create) {
    auto it = symbols.find(symbol);

    if (it != symbols.end()) findSymbol(*parent, *this, symbol, parameters, arguments);

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