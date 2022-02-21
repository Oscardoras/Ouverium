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
                for (size_t i = 0; i < tuple->objects.size(); i++)
                    setReferences(context, tuple->objects[i], Reference(&object->data.tuple[i]));
            } else throw InterpreterError();
        }

    }
}

void freeContext(Context & context, Reference & result) {
    for (auto & symbol : context.symbols)
        if (symbol.second.isCopy()) {
            int r = checkObject(symbol.second.ptrCopy, result);
            if (r == 2) freeContext(symbol.second.ptrCopy, result);
            else if (r == 1) symbol.second.ptrCopy->references--;
        }
}

void freeContext(Context & context) {
    for (auto & symbol : context.symbols)
        if (symbol.second.isCopy())
            symbol.second.ptrCopy->removeReference();
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
            tmp.unuse();
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
            if (func->function->type == Function::Custom) {
                FunctionContext funcContext(context);
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
                    freeContext(funcContext, result);
                    if (func->references == 0) delete func;
                    if (filter->references == 0) delete filter;
                    return result;
                } else {
                    freeContext(funcContext);
                    if (func->references == 0) delete func;
                    if (filter->references == 0) delete filter;
                    throw InterpreterError();
                }
            } else if (func->function->type == Function::System) {
                auto arguments = execute(context, functionCall->object);
                auto result = ((SystemFunction*) func->function)->pointer(arguments);
                if (func->references == 0) delete func;
                return result;
            } else {
                if (func->references == 0) delete func;
                throw InterpreterError();
            }
        } else {
            if (func->references == 0) delete func;
            throw InterpreterError();
        }
    } else if (type == "FunctionDefinition") {
        auto functionDefinition = std::static_pointer_cast<FunctionDefinition>(expression);

        auto object = new Object();
        object->function = new CustomFunction(functionDefinition);
        if (context.global != &context)
            for (auto symbol : functionDefinition->object->usedSymbols)
                if (context.hasSymbol(symbol)) {
                    auto obj = context.getSymbol(symbol).toObject();
                    ((CustomFunction*) object->function)->objects[symbol] = obj;
                    obj->addReference();
                }
        
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
            field = nullptr;
            delete object;
            return ref;
        } else return Reference(&field);
    } else if (type == "Symbol") {
        auto symbol = std::static_pointer_cast<Symbol>(expression)->name;
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

void addFunction(Context & context, std::string symbol, Reference (*function)(Reference)) {
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

    auto ref = execute(context, expression);
    SystemFunctions::print(ref).unuse();
}