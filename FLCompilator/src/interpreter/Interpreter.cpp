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


Reference Interpreter::callFunction(Context & context, Function* function, std::shared_ptr<Expression> arguments) {
    if (function->type == Function::Custom) {
        FunctionContext function_context(context, ((CustomFunction*) function)->pointer->parameters, arguments);

        for (auto & symbol : ((CustomFunction*) function)->objects)
            function_context.addSymbol(symbol.first, Reference(symbol.second));

        Object* filter;
        if (((CustomFunction*) function)->pointer->filter != nullptr)
            filter = execute(function_context, ((CustomFunction*) function)->pointer->filter).toObject();
        else filter = context.addObject(new Object(true));

        if (filter->type == Object::Boolean && filter->data.b)
            return execute(function_context, ((CustomFunction*) function)->pointer->object);
        else throw InterpreterError();
    } else {
        FunctionContext function_context(context, ((SystemFunction*) function)->pointer->parameters, arguments);
        
        return ((SystemFunction*) function)->pointer(function_context);
    }
}


Reference Interpreter::execute(Context & context, std::shared_ptr<Expression> expression) {
    if (expression->type == Expression::FunctionCall) {
        auto function_call = std::static_pointer_cast<FunctionCall>(expression);

        auto func = execute(context, function_call->function).toObject();
        if (func->function != nullptr)
            return callFunction(context, func->function, function_call->object);
    } else if (expression->type == Expression::FunctionDefinition) {
        auto function_definition = std::static_pointer_cast<FunctionDefinition>(expression);

        auto object = context.addObject(new Object());
        object->function = new CustomFunction(function_definition);
        if (context.getGlobal() != &context)
            for (auto symbol : function_definition->object->usedSymbols)
                if (context.hasSymbol(symbol))
                    ((CustomFunction*) object->function)->objects[symbol] = context.getSymbol(symbol).toObject();
        
        return Reference(object);
    } else if (expression->type == Expression::Property) {
        auto property = std::static_pointer_cast<Property>(expression);
        
        auto object = execute(context, property->object).toObject();
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


void addFunction(Context & context, std::string symbol, Reference (*function)(Reference, FunctionContext&)) {
    Object * object = new Object();
    object->function = new SystemFunction(function);
    context.addSymbol(symbol, Reference(object));
}

void Interpreter::run(std::shared_ptr<Expression> expression) {
    GlobalContext context;
    addFunction(context, ";", SystemFunctions::separator);
    addFunction(context, "$", SystemFunctions::copy);
    addFunction(context, ":=", SystemFunctions::assign);
    addFunction(context, ":", SystemFunctions::function_definition);
    addFunction(context, "=", SystemFunctions::equals);
    addFunction(context, "!=", SystemFunctions::not_equals);
    addFunction(context, "===", SystemFunctions::checkPointers);
    addFunction(context, "!", SystemFunctions::logicalNot);
    addFunction(context, "&", SystemFunctions::logicalAnd);
    addFunction(context, "|", SystemFunctions::logicalOr);
    addFunction(context, "+", SystemFunctions::addition);
    addFunction(context, "-", SystemFunctions::substraction);
    addFunction(context, "*", SystemFunctions::multiplication);
    addFunction(context, "/", SystemFunctions::division);
    addFunction(context, "%", SystemFunctions::modulo);
    addFunction(context, "print", SystemFunctions::print);
    addFunction(context, "lenght", SystemFunctions::get_array_size);
    addFunction(context, "get_capacity", SystemFunctions::get_array_capacity);
    addFunction(context, "set_capacity", SystemFunctions::set_array_capacity);
    addFunction(context, "get", SystemFunctions::get_array_element);
    addFunction(context, "add", SystemFunctions::add_array_element);
    addFunction(context, "remove", SystemFunctions::add_array_element);

    auto result = execute(context, expression);
    SystemFunction print(SystemFunctions::print);
    callFunction(context, &print, std::shared_ptr<Expression> arguments);
}