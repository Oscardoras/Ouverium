#include <stdexcept>

#include "Context.hpp"
#include "Interpreter.hpp"
#include "InterpreterError.hpp"
#include "SystemFunctions.cpp"

#include "../parser/expression/Condition.hpp"
#include "../parser/expression/ConditionAlternative.hpp"
#include "../parser/expression/ConditionRepeat.hpp"
#include "../parser/expression/Expression.hpp"
#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"


void setReferences(Context & context, std::shared_ptr<Expression> expression, Reference reference) {
    auto type = expression->getType();

    if (type == "Symbol") {
        auto symbol = std::static_pointer_cast<Symbol>(expression);

        context.addSymbol(symbol->name, reference);
    } else if (type == "Tuple") {
        auto tuple = std::static_pointer_cast<Tuple>(expression);

        if (reference.isTuple()) {
            for (size_t i = 0; i < tuple->objects.size(); i++)
                setReferences(context, tuple->objects[i], reference.tuple[i]);
        } else {
            auto object = reference.toObject();
            if (object->type >= 0) {
                for (unsigned long i = 1; i <= tuple->objects.size(); i++)
                    setReferences(context, tuple->objects[i], Reference(&object->data.a[i].o));
            } else throw InterpreterError();
        }

    }
}


Reference execute(Context & context, std::shared_ptr<Expression> expression) {
    auto type = expression->getType();

    if (type == "Condition") {
        auto condition = std::static_pointer_cast<Condition>(expression);

        auto condition_val = execute(context, condition->condition).toObject();
        if (condition_val->type == Object::Boolean) {
            if (condition_val->data.b) {
                if (condition_val->references == 0) delete condition_val;
                return execute(context, condition->object);
            } else {
                if (condition_val->references == 0) delete condition_val;
                return Reference(new Object());
            }
        } else {
            if (condition_val->references == 0) delete condition_val;
            throw InterpreterError();
        }
    } else if (type == "ConditionAlternative") {
        auto conditionAlternative = std::static_pointer_cast<ConditionAlternative>(expression);

        auto condition_val = execute(context, conditionAlternative->condition).toObject();
        if (condition_val->type == Object::Boolean) {
            if (condition_val->data.b) {
                if (condition_val->references == 0) delete condition_val;
                return execute(context, conditionAlternative->object);
            } else {
                if (condition_val->references == 0) delete condition_val;
                return execute(context, conditionAlternative->alternative);
            }
        } else {
            if (condition_val->references == 0) delete condition_val;
            throw InterpreterError();
        }
    } else if (type == "ConditionRepeat") {
        auto conditionRepeat = std::static_pointer_cast<ConditionRepeat>(expression);

        Reference tmp(new Object());
        auto condition_val = execute(context, conditionRepeat->condition).toObject();
        if (condition_val->type != Object::Boolean) {
            if (condition_val->references == 0) delete condition_val;
            throw InterpreterError();
        }
        while (condition_val->data.b) {
            if (condition_val->references == 0) delete condition_val;
            context.unuse(tmp);
            tmp = execute(context, conditionRepeat->object);
            condition_val = execute(context, conditionRepeat->condition).toObject();
            if (condition_val->type != Object::Boolean) {
                if (condition_val->references == 0) delete condition_val;
                throw InterpreterError();
            }
        }
        if (condition_val->references == 0) delete condition_val;
        return tmp;
    } else if (type == "FunctionCall") {
        auto functionCall = std::static_pointer_cast<FunctionCall>(expression);

        auto func = execute(context, functionCall->function).toObject();
        if (func->function != nullptr) {
            FunctionContext funcContext(context);
            if (func->function->type == Function::Custom) {
                auto parameters = ((CustomFunction*) func->function)->pointer->parameters;
                auto arguments = execute(context, functionCall->object);
                for (auto & symbol : ((CustomFunction*) func->function)->objects)
                    funcContext.addSymbol(symbol.first, Reference(symbol.second));
                setReferences(funcContext, parameters, arguments);

                Object* filter;
                if (((CustomFunction*) func->function)->pointer->filter != nullptr)
                    filter = execute(funcContext, ((CustomFunction*) func->function)->pointer->filter).toObject();
                else filter = new Object(true);

                if (filter->type == Object::Boolean && filter->data.b) {
                    auto result = execute(funcContext, ((CustomFunction*) func->function)->pointer->object);
                    funcContext.freeContext(result);
                    if (func->references == 0) context.free(func);
                    if (filter->references == 0) context.free(filter);
                    return result;
                } else {
                    funcContext.freeContext();
                    if (func->references == 0) context.free(func);
                    if (filter->references == 0) context.free(filter);
                    throw InterpreterError();
                }
            } else if (func->function->type == Function::System) {
                auto arguments = execute(context, functionCall->object);
                funcContext.addSymbol("arguments", arguments);

                try {
                    auto result = ((SystemFunction*) func->function)->pointer(arguments, funcContext);
                    funcContext.freeContext(result);
                    if (func->references == 0) context.free(func);
                    return result;
                } catch (InterpreterError & ex) {
                    funcContext.freeContext();
                    if (func->references == 0) context.free(func);
                    throw ex;
                }
            } else {
                if (func->references == 0) context.free(func);
                throw InterpreterError();
            }
        } else {
            if (func->references == 0) context.free(func);
            throw InterpreterError();
        }
    } else if (type == "FunctionDefinition") {
        auto functionDefinition = std::static_pointer_cast<FunctionDefinition>(expression);

        auto object = new Object();
        object->function = new CustomFunction(functionDefinition);
        if (context.global != &context)
            for (auto symbol : functionDefinition->object->usedSymbols)
                if (context.hasSymbol(symbol))
                    ((CustomFunction*) object->function)->objects[symbol] = context.getSymbol(symbol).toObject();
        
        return Reference(object);
    } else if (type == "Property") {
        auto property = std::static_pointer_cast<Property>(expression);
        
        auto object = execute(context, property->object).toObject();
        auto & field = object->fields[property->name];
        if (field == nullptr) {
            field = new Object();
            if (object->references > 0) field->references++;
        }

        if (object->references == 0) {
            auto ref = Reference(field);
            field = new Object();
            delete object;
            return ref;
        } else return Reference(&field);
    } else if (type == "Symbol") {
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
            Object* obj = new Object((size_t) l);
            for (long i = 0; i < l; i++)
                obj->data.a[i+1].o = new Object(str[i]);
            return obj;
        }
        if (symbol == "true") return Reference(new Object(true));
        if (symbol == "false") return Reference(new Object(false));
        try {
            return Reference(new Object(std::stol(symbol)));
        } catch (std::invalid_argument const& ex1) {
            try {
                return Reference(new Object(std::stod(symbol)));
            } catch (std::invalid_argument const& ex2) {
                return context.getSymbol(std::static_pointer_cast<Symbol>(expression)->name);
            }
        }
    } else if (type == "Tuple") {
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
    
    FunctionContext funcContext(context);
    funcContext.addSymbol("arguments", result);
    
    auto ref = SystemFunctions::print(result, funcContext);
    context.unuse(ref);

    funcContext.freeContext();
}