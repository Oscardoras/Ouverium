#include <stdexcept>

#include "Function.hpp"
#include "Interpreter.hpp"

#include "system_functions/Array.cpp"
#include "system_functions/Base.cpp"
#include "system_functions/Math.cpp"
#include "system_functions/Streams.cpp"
#include "system_functions/Types.cpp"

#include "../parser/expression/Expression.hpp"
#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"

#include "../parser/Standard.hpp"



void setReferences(FunctionContext & function_context, std::shared_ptr<Expression> parameters, Reference reference) {
    if (parameters->type == Expression::Symbol) {
        auto symbol = std::static_pointer_cast<Symbol>(parameters);

        function_context.addSymbol(symbol->name, reference);
    } else if (parameters->type == Expression::Tuple) {
        auto tuple = std::static_pointer_cast<Tuple>(parameters);

        if (reference.type > 0) {
            for (size_t i = 0; i < tuple->objects.size(); i++)
                setReferences(function_context, tuple->objects[i], reference.tuple[i]);
        } else {
            auto object = reference.toObject(function_context);
            if (object->type >= 0) {
                for (unsigned long i = 1; i <= tuple->objects.size(); i++)
                    setReferences(function_context, tuple->objects[i], Reference(&object->data.a[i].o));
            } else throw FunctionArgumentsError();
        }
    } else throw FunctionArgumentsError();
}

void setReferences(Context & context, FunctionContext & function_context, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
    if (parameters->type == Expression::Symbol) {
        auto symbol = std::static_pointer_cast<Symbol>(parameters);

        function_context.addSymbol(symbol->name, Interpreter::execute(context, arguments));
    } else if (parameters->type == Expression::Tuple) {
        auto p_tuple = std::static_pointer_cast<Tuple>(parameters);

        if (arguments->type == Expression::Tuple) {
            auto a_tuple = std::static_pointer_cast<Tuple>(arguments);

            if (p_tuple->objects.size() == a_tuple->objects.size()) {
                size_t size = p_tuple->objects.size();
                for (size_t i = 0; i < size; i++)
                    setReferences(context, function_context, p_tuple->objects[i], a_tuple->objects[i]);
            } else throw FunctionArgumentsError();
        } else setReferences(function_context, parameters, Interpreter::execute(context, arguments));
    } else if (parameters->type == Expression::FunctionCall) {
        auto p_function = std::static_pointer_cast<FunctionCall>(parameters);

        if (p_function->function->type == Expression::Symbol) {
            auto symbol = std::static_pointer_cast<Symbol>(p_function->function);

            auto function_definition = std::make_shared<FunctionDefinition>();
            function_definition->parameters = p_function->object;
            function_definition->object = arguments;

            auto object = context.newObject();
            auto f = new CustomFunction(function_definition);
            for (auto symbol : function_definition->object->usedSymbols)
                ((CustomFunction*) f)->externSymbols[symbol] = context.getSymbol(symbol);
            object->functions.push_front(f);

            function_context.addSymbol(symbol->name, Reference(object));
        } else throw FunctionArgumentsError();
    } else throw FunctionArgumentsError();
}

Reference Interpreter::callFunction(Context & context, std::list<Function*> functions, std::shared_ptr<Expression> arguments, std::shared_ptr<Position> position) {
    for (auto function : functions) {
        try {
            FunctionContext function_context(context);
            if (position != nullptr) function_context.addSymbol("system_position", context.newObject(position->path));

            for (auto & symbol : function->externSymbols)
                function_context.addSymbol(symbol.first, symbol.second);

            if (function->type == Function::Custom) {
                setReferences(context, function_context, ((CustomFunction*) function)->pointer->parameters, arguments);

                Object* filter;
                if (((CustomFunction*) function)->pointer->filter != nullptr)
                    filter = execute(function_context, ((CustomFunction*) function)->pointer->filter).toObject(context);
                else filter = context.newObject(true);

                if (filter->type == Object::Bool && filter->data.b)
                    return execute(function_context, ((CustomFunction*) function)->pointer->object);
                else throw FunctionArgumentsError();
            } else {
                setReferences(context, function_context, ((SystemFunction*) function)->parameters, arguments);
                
                return ((SystemFunction*) function)->pointer(function_context);
            }

        } catch (FunctionArgumentsError & error) {}
    }

    if (arguments->position != nullptr) arguments->position->notify();
    throw InterpreterError();
}


Reference Interpreter::execute(Context & context, std::shared_ptr<Expression> expression) {
    if (expression->type == Expression::FunctionCall) {
        auto function_call = std::static_pointer_cast<FunctionCall>(expression);

        auto func = execute(context, function_call->function).toObject(context);
        return callFunction(context, func->functions, function_call->object, function_call->position);
    } else if (expression->type == Expression::FunctionDefinition) {
        auto function_definition = std::static_pointer_cast<FunctionDefinition>(expression);

        auto object = context.newObject();
        auto f = new CustomFunction(function_definition);
        for (auto symbol : function_definition->object->usedSymbols)
            if (context.hasSymbol(symbol))
                f->externSymbols[symbol] = context.getSymbol(symbol);
        if (function_definition->filter != nullptr)
            for (auto symbol : function_definition->filter->usedSymbols)
                if (context.hasSymbol(symbol))
                    f->externSymbols[symbol] = context.getSymbol(symbol);
        object->functions.push_front(f);
        
        return Reference(object);
    } else if (expression->type == Expression::Property) {
        auto property = std::static_pointer_cast<Property>(expression);
        
        auto object = execute(context, property->object).toObject(context);
        auto & field = object->fields[property->name];
        if (field == nullptr) field = context.newObject();
        return Reference(object, &field);
    } else if (expression->type == Expression::Symbol) {
        auto symbol = std::static_pointer_cast<Symbol>(expression)->name;

        if (symbol[0] == '\"') {
            std::string str;

            bool escape = false;
            bool first = true;
            for (char c : symbol) if (!first) {
                if (!escape) {
                    if (c == '\"') break;
                    else if (c == '\\') escape = true;
                    else str += c;
                } else {
                    escape = false;
                    if (c == 'b') str += '\b';
                    if (c == 'e') str += '\e';
                    if (c == 'f') str += '\f';
                    if (c == 'n') str += '\n';
                    if (c == 'r') str += '\r';
                    if (c == 't') str += '\t';
                    if (c == 'v') str += '\v';
                    if (c == '\\') str += '\\';
                    if (c == '\'') str += '\'';
                    if (c == '\"') str += '\"';
                    if (c == '?') str += '\?';
                }
            } else first = false;

            return context.newObject(str);
        }
        if (symbol == "true") return Reference(context.newObject(true));
        if (symbol == "false") return Reference(context.newObject(false));
        try {
            return Reference(context.newObject(std::stol(symbol)));
        } catch (std::invalid_argument const& ex1) {
            try {
                return Reference(context.newObject(std::stod(symbol)));
            } catch (std::invalid_argument const& ex2) {
                return context.getSymbol(symbol);
            }
        }
    } else if (expression->type == Expression::Tuple) {
        auto tuple = std::static_pointer_cast<Tuple>(expression);

        auto n = tuple->objects.size();
        Reference reference;
        if (n > 0) {
            reference = Reference(n);
            for (int i = 0; i < (int) n; i++)
                reference.tuple[i] = execute(context, tuple->objects[i]);
        } else reference = Reference(context.newObject());
        return reference;
    } else return Reference();
}


void addFunction(Context & context, std::string symbol, std::shared_ptr<Expression> parameters, Reference (*function)(FunctionContext&)) {
    context.getSymbol(symbol).toObject(context)->functions.push_front(new SystemFunction(parameters, function));
}

void Interpreter::setStandardContext(Context & context) {
    Array::initiate(context);
    Base::initiate(context);
    Math::initiate(context);
    Streams::initiate(context);
    Types::initiate(context);
}

Reference Interpreter::run(Context & context, std::string const& path, std::string const& code) {
    std::vector<std::string> symbols;
    for (auto it = context.symbols.begin(); it != context.symbols.end(); it++)
        symbols.push_back(it->first);

    try {
        auto tree = StandardParser::getTree(path, code, symbols);
        //std::cout << tree->toString() << std::endl;
        try {
            return Interpreter::execute(context, tree);
        } catch (InterpreterError & e) {
            return Reference(context.newObject());
        }
    } catch (StandardParser::ParserError & e) {
        std::cerr << e.message << " in file \"" << e.position.path << "\" at line " << e.position.line << ", column " << e.position.column << "." << std::endl;
        return Reference(context.newObject());
    }
}

bool Interpreter::print(std::ostream & stream, Object* object) {
    if (object->type == Object::Bool) {
        stream << object->data.b;
        return true;
    } else if (object->type == Object::Int) {
        stream << object->data.i;
        return true;
    } else if (object->type == Object::Float) {
        stream << object->data.f;
        return true;
    } else if (object->type == Object::Char) {
        stream << object->data.c;
        return true;
    } else if (object->type > 0) {
        bool printed = false;
        for (long i = 1; i <= object->type; i++)
            if (print(stream, object->data.a[i].o)) printed = true;
        return printed;
    } else return false;
}