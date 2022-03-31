#include <stdexcept>

#include "Context.hpp"
#include "Interpreter.hpp"
#include "InterpreterError.hpp"
#include "SystemFunctions.cpp"

#include "../parser/expression/Expression.hpp"
#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"



void setReferences(FunctionContext & function_context, std::shared_ptr<Expression> parameters, Reference reference) {
    if (parameters->type == Expression::Symbol) {
        auto symbol = std::static_pointer_cast<Symbol>(parameters);

        function_context.addSymbol(symbol->name, reference);
    } else if (parameters->type == Expression::Tuple) {
        auto tuple = std::static_pointer_cast<Tuple>(parameters);

        if (reference.isTuple()) {
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
                for (size_t i = 0; i < p_tuple->objects.size(); i++)
                    setReferences(context, function_context, p_tuple->objects[i], a_tuple->objects[i]);
            } else throw FunctionArgumentsError();
        } else setReferences(function_context, parameters, Interpreter::execute(context, arguments));
    } else if (parameters->type == Expression::FunctionCall) {
        auto p_function = std::static_pointer_cast<FunctionCall>(parameters);

        if (p_function->function->type == Expression::Symbol) {
            auto symbol = std::static_pointer_cast<Symbol>(p_function->function);

            if (context.hasSymbol(symbol->name)) {
                //TODO
            } else {
                auto function = std::make_shared<FunctionDefinition>();
                function->parameters = p_function->object;
                function->object = arguments;
                function_context.addSymbol(symbol->name, Interpreter::execute(context, function));
            }
        } else throw FunctionArgumentsError();
    } else throw FunctionArgumentsError();
}

Reference Interpreter::callFunction(Context & context, std::list<Function*> functions, std::shared_ptr<Expression> arguments) {
    for (auto function : functions) {
        try {
            if (function->type == Function::Custom) {
                FunctionContext function_context(context);

                for (auto & symbol : ((CustomFunction*) function)->objects)
                    function_context.addSymbol(symbol.first, Reference(symbol.second));
                
                setReferences(context, function_context, ((CustomFunction*) function)->pointer->parameters, arguments);

                Object* filter;
                if (((CustomFunction*) function)->pointer->filter != nullptr)
                    filter = execute(function_context, ((CustomFunction*) function)->pointer->filter).toObject(context);
                else filter = context.addObject(new Object(true));

                if (filter->type == Object::Boolean && filter->data.b)
                    return execute(function_context, ((CustomFunction*) function)->pointer->object);
                else throw FunctionArgumentsError();
            } else {
                FunctionContext function_context(context);

                setReferences(context, function_context, ((CustomFunction*) function)->pointer->parameters, arguments);
                
                return ((SystemFunction*) function)->pointer(function_context);
            }
        } catch (FunctionArgumentsError error) {}
    }

    throw InterpreterError();
}


Reference Interpreter::execute(Context & context, std::shared_ptr<Expression> expression) {
    if (expression->type == Expression::FunctionCall) {
        auto function_call = std::static_pointer_cast<FunctionCall>(expression);

        auto func = execute(context, function_call->function).toObject(context);
        return callFunction(context, func->functions, function_call->object);
    } else if (expression->type == Expression::FunctionDefinition) {
        auto function_definition = std::static_pointer_cast<FunctionDefinition>(expression);

        auto object = context.addObject(new Object());
        auto f = new CustomFunction(function_definition);
        if (context.getGlobal() != &context)
            for (auto symbol : function_definition->object->usedSymbols)
                if (context.hasSymbol(symbol))
                    ((CustomFunction*) f)->objects[symbol] = context.getSymbol(symbol).toObject(context);
        object->functions.push_front(f);
        
        return Reference(object);
    } else if (expression->type == Expression::Property) {
        auto property = std::static_pointer_cast<Property>(expression);
        
        auto object = execute(context, property->object).toObject(context);
        auto & field = object->fields[property->name];
        if (field == nullptr) field = context.addObject(new Object());
        return Reference(&field);
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

            long l = str.length();
            Object* obj = context.addObject(new Object((size_t) l));
            for (long i = 0; i < l; i++)
                obj->data.a[i+1].o = context.addObject(new Object(str[i]));
            return obj;
        }
        if (symbol == "true") return Reference(context.addObject(new Object(true)));
        if (symbol == "false") return Reference(context.addObject(new Object(false)));
        try {
            return Reference(context.addObject(new Object(std::stol(symbol))));
        } catch (std::invalid_argument const& ex1) {
            try {
                return Reference(context.addObject(new Object(std::stod(symbol))));
            } catch (std::invalid_argument const& ex2) {
                return context.getSymbol(std::static_pointer_cast<Symbol>(expression)->name);
            }
        }
    } else if (expression->type == Expression::Tuple) {
        auto tuple = std::static_pointer_cast<Tuple>(expression);

        auto n = tuple->objects.size();
        Reference reference(n);
        for (int i = 0; i < (int) n; i++)
            reference.tuple[i] = execute(context, tuple->objects[i]);
        return reference;
    } else return Reference();
}


void addFunction(Context & context, std::string symbol, std::shared_ptr<Expression> parameters, Reference (*function)(FunctionContext&)) {
    context.getSymbol(symbol).toObject(context)->functions.push_front(new SystemFunction(parameters, function));
}

void Interpreter::run(std::shared_ptr<Expression> expression) {
    GlobalContext context;
    addFunction(context, ";", SystemFunctions::separator(), SystemFunctions::separator);
    addFunction(context, "$", SystemFunctions::copy(), SystemFunctions::copy);
    addFunction(context, ":=", SystemFunctions::assign(), SystemFunctions::assign);
    addFunction(context, ":", SystemFunctions::function_definition(), SystemFunctions::function_definition);
    addFunction(context, "=", SystemFunctions::equals(), SystemFunctions::equals);
    addFunction(context, "!=", SystemFunctions::not_equals(), SystemFunctions::not_equals);
    addFunction(context, "===", SystemFunctions::check_pointers(), SystemFunctions::check_pointers);
    addFunction(context, "!", SystemFunctions::logical_not(), SystemFunctions::logical_not);
    addFunction(context, "&", SystemFunctions::logical_and(), SystemFunctions::logical_and);
    addFunction(context, "|", SystemFunctions::logical_or(), SystemFunctions::logical_or);
    addFunction(context, "+", SystemFunctions::addition(), SystemFunctions::addition);
    addFunction(context, "-", SystemFunctions::opposite(), SystemFunctions::opposite);
    addFunction(context, "-", SystemFunctions::substraction(), SystemFunctions::substraction);
    addFunction(context, "*", SystemFunctions::multiplication(), SystemFunctions::multiplication);
    addFunction(context, "/", SystemFunctions::division(), SystemFunctions::division);
    addFunction(context, "%", SystemFunctions::modulo(), SystemFunctions::modulo);
    addFunction(context, "print", SystemFunctions::print(), SystemFunctions::print);
    addFunction(context, "lenght", SystemFunctions::get_array_size(), SystemFunctions::get_array_size);
    addFunction(context, "get_capacity", SystemFunctions::get_array_capacity(), SystemFunctions::get_array_capacity);
    addFunction(context, "set_capacity", SystemFunctions::set_array_capacity(), SystemFunctions::set_array_capacity);
    addFunction(context, "get", SystemFunctions::get_array_element(), SystemFunctions::get_array_element);
    addFunction(context, "add", SystemFunctions::add_array_element(), SystemFunctions::add_array_element);
    addFunction(context, "remove", SystemFunctions::add_array_element(), SystemFunctions::add_array_element);

    auto result = execute(context, expression).toObject(context);
    SystemFunctions::print(result);
}