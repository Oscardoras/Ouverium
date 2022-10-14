#include <stdexcept>
#include <iostream>

#include "Function.hpp"
#include "Interpreter.hpp"

#include "system_functions/Array.hpp"
#include "system_functions/ArrayList.hpp"
#include "system_functions/Base.hpp"
#include "system_functions/Math.hpp"
#include "system_functions/Streams.hpp"
#include "system_functions/String.hpp"
#include "system_functions/Types.hpp"

#include "../parser/expression/Expression.hpp"
#include "../parser/expression/FunctionCall.hpp"
#include "../parser/expression/FunctionDefinition.hpp"
#include "../parser/expression/Property.hpp"
#include "../parser/expression/Symbol.hpp"
#include "../parser/expression/Tuple.hpp"

#include "../parser/Standard.hpp"


GlobalContext::GlobalContext() {
    Array::init(*this);
    ArrayList::init(*this);
    Base::init(*this);
    Math::init(*this);
    Streams::init(*this);
    String::init(*this);
    Types::init(*this);
}


void set_references(FunctionContext & function_context, std::shared_ptr<Expression> parameters, Reference const& reference) {
    if (parameters->type == Expression::Symbol) {
        auto symbol = std::static_pointer_cast<Symbol>(parameters);

        function_context.add_symbol(symbol->name, reference);
    } else if (parameters->type == Expression::Tuple) {
        auto tuple = std::static_pointer_cast<Tuple>(parameters);

        if (reference.type == (long) tuple->objects.size()) {
            for (size_t i = 0; i < tuple->objects.size(); i++)
                set_references(function_context, tuple->objects[i], reference.tuple[i]);
        } else {
            auto object = reference.to_object(function_context);
            if (object->type == (long) tuple->objects.size()) {
                for (unsigned long i = 1; i <= tuple->objects.size(); i++)
                    set_references(function_context, tuple->objects[i], Reference(&object->data.a[i].o));
            } else throw Interpreter::FunctionArgumentsError();
        }
    } else throw Interpreter::FunctionArgumentsError();
}

void set_references(Context & context, FunctionContext & function_context, std::map<std::shared_ptr<Expression>, Reference> & computed, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
    if (parameters->type == Expression::Symbol) {
        auto symbol = std::static_pointer_cast<Symbol>(parameters);

        auto it = computed.find(arguments);
        auto reference = it != computed.end() ? it->second : (computed[arguments] = Interpreter::execute(context, arguments));
        function_context.add_symbol(symbol->name, reference);
    } else if (parameters->type == Expression::Tuple) {
        auto p_tuple = std::static_pointer_cast<Tuple>(parameters);

        if (arguments->type == Expression::Tuple) {
            auto a_tuple = std::static_pointer_cast<Tuple>(arguments);

            if (p_tuple->objects.size() == a_tuple->objects.size()) {
                size_t size = p_tuple->objects.size();
                for (size_t i = 0; i < size; i++)
                    set_references(context, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]);
            } else throw Interpreter::FunctionArgumentsError();
        } else {
            auto it = computed.find(arguments);
            auto reference = it != computed.end() ? it->second : (computed[arguments] = Interpreter::execute(context, arguments));
            set_references(function_context, parameters, reference);
        }
    } else if (parameters->type == Expression::FunctionCall) {
        auto p_function = std::static_pointer_cast<FunctionCall>(parameters);

        if (p_function->function->type == Expression::Symbol) {
            auto it = computed.find(arguments);
            if (it != computed.end())
                throw Interpreter::FunctionArgumentsError();

            auto symbol = std::static_pointer_cast<Symbol>(p_function->function);

            auto function_definition = std::make_shared<FunctionDefinition>();
            function_definition->parameters = p_function->object;
            function_definition->object = arguments;

            auto object = context.new_object();
            auto f = new CustomFunction(function_definition);
            for (auto symbol : function_definition->object->symbols)
                ((CustomFunction*) f)->extern_symbols[symbol] = context.get_symbol(symbol);
            object->functions.push_front(f);

            function_context.add_symbol(symbol->name, Reference(object));
        } else throw Interpreter::FunctionArgumentsError();
    } else throw Interpreter::FunctionArgumentsError();
}

Reference Interpreter::call_function(Context & context, std::shared_ptr<Position> position, std::list<Function*> function, Reference const& reference) {
    Object* object = reference.to_object(context);

    for (auto const& function : function) {
        try {
            FunctionContext function_context(context, nullptr);
            for (auto & symbol : function->extern_symbols)
                function_context.add_symbol(symbol.first, symbol.second);

            if (function->type == Function::Custom) {
                set_references(function_context, ((CustomFunction*) function)->pointer->parameters, object);

                Object* filter;
                if (((CustomFunction*) function)->pointer->filter != nullptr)
                    filter = Interpreter::execute(function_context, ((CustomFunction*) function)->pointer->filter).to_object(context);
                else filter = nullptr;

                if (filter == nullptr || (filter->type == Object::Bool && filter->data.b))
                    return Interpreter::execute(function_context, ((CustomFunction*) function)->pointer->object);
                else continue;
            } else {
                set_references(function_context, ((SystemFunction*) function)->parameters, object);

                return ((SystemFunction*) function)->pointer(function_context);
            }
        } catch (FunctionArgumentsError & e) {}
    }

    if (position != nullptr) {
        position->store_stack_trace(context);
        position->notify_error();
    }
    throw Error();
}

Reference Interpreter::call_function(Context & context, std::shared_ptr<Position> position, std::list<Function*> functions, std::shared_ptr<Expression> arguments) {
    std::map<std::shared_ptr<Expression>, Reference> computed;

    for (auto const& function : functions) {
        try {
            FunctionContext function_context(context, position);
            if (position != nullptr) function_context.add_symbol("system_position", context.new_object(position->path));
            for (auto & symbol : function->extern_symbols)
                function_context.add_symbol(symbol.first, symbol.second);

            if (function->type == Function::Custom) {
                set_references(context, function_context, computed, ((CustomFunction*) function)->pointer->parameters, arguments);

                Object* filter;
                if (((CustomFunction*) function)->pointer->filter != nullptr)
                    filter = execute(function_context, ((CustomFunction*) function)->pointer->filter).to_object(context);
                else filter = nullptr;

                if (filter == nullptr || (filter->type == Object::Bool && filter->data.b))
                    return execute(function_context, ((CustomFunction*) function)->pointer->object);
                else throw FunctionArgumentsError();
            } else {
                set_references(context, function_context, computed, ((SystemFunction*) function)->parameters, arguments);

                return ((SystemFunction*) function)->pointer(function_context);
            }

        } catch (FunctionArgumentsError & e) {}
    }

    if (position != nullptr) {
        position->store_stack_trace(context);
        position->notify_error();
    }
    throw Error();
}


Reference Interpreter::execute(Context & context, std::shared_ptr<Expression> expression) {
    if (expression->type == Expression::FunctionCall) {
        auto function_call = std::static_pointer_cast<FunctionCall>(expression);

        auto func = execute(context, function_call->function).to_object(context);
        return call_function(context, function_call->position, func->functions, function_call->object);
    } else if (expression->type == Expression::FunctionDefinition) {
        auto function_definition = std::static_pointer_cast<FunctionDefinition>(expression);
        auto object = context.new_object();
        auto f = new CustomFunction(function_definition);
        for (auto symbol : function_definition->object->symbols)
            if (context.has_symbol(symbol))
                f->extern_symbols[symbol] = context.get_symbol(symbol);
        if (function_definition->filter != nullptr)
            for (auto symbol : function_definition->filter->symbols)
                if (context.has_symbol(symbol))
                    f->extern_symbols[symbol] = context.get_symbol(symbol);
        object->functions.push_front(f);

        return Reference(object);
    } else if (expression->type == Expression::Property) {
        auto property = std::static_pointer_cast<Property>(expression);

        auto object = execute(context, property->object).to_object(context);
        return Reference(object, &object->get_property(property->name, context));
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

            return context.new_object(str);
        }
        if (symbol == "true") return Reference(context.new_object(true));
        if (symbol == "false") return Reference(context.new_object(false));
        try {
            return Reference(context.new_object(std::stol(symbol)));
        } catch (std::invalid_argument const& ex1) {
            try {
                return Reference(context.new_object(std::stod(symbol)));
            } catch (std::invalid_argument const& ex2) {
                return context.get_symbol(symbol);
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
        } else reference = Reference(context.new_object());
        return reference;
    } else return Reference();
}


Reference Interpreter::run(Context & context, std::shared_ptr<Expression> expression) {
    try {
        return Interpreter::execute(context, expression);
    } catch (Base::Exception & ex) {
        ex.position->notify_error();
        return Reference(context.new_object());
    } catch (Error & e) {
        return Reference(context.new_object());
    }
}

bool Interpreter::print(std::ostream & stream, Object* object) {
    if (object->type == Object::Bool) {
        std::cout << object->data.b;
        return true;
    } else if (object->type == Object::Int) {
        std::cout << object->data.i;
        return true;
    } else if (object->type == Object::Float) {
        std::cout << object->data.f;
        return true;
    } else if (object->type == Object::Char) {
        std::cout << object->data.c;
        return true;
    } else if (object->type > 0) {
        bool printed = false;
        for (long i = 1; i <= object->type; i++)
            if (print(stream, object->data.a[i].o)) printed = true;
        return printed;
    } else return false;
}
