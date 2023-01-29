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

#include "../parser/Standard.hpp"

#include "../Utils.hpp"


namespace Interpreter {

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
        if (auto symbol = std::dynamic_pointer_cast<Symbol>(parameters)) {
            function_context.add_symbol(symbol->name, reference);
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(parameters)) {
            if (auto tuple_reference = std::get_if<TupleReference>(&reference)) {
                if (tuple_reference->size() == tuple->objects.size()) {
                    for (size_t i = 0; i < tuple->objects.size(); i++)
                        set_references(function_context, tuple->objects[i], tuple_reference[i]);
                } else throw Interpreter::FunctionArgumentsError();
            } else {
                auto data = reference.to_data(function_context);
                if (auto object = std::get_if<Object*>(&data)) {
                    if ((*object)->array.size() == tuple->objects.size()) {
                        for (size_t i = 0; i < tuple->objects.size(); i++)
                            set_references(function_context, tuple->objects[i], ArrayReference{**object, i});
                    } else throw Interpreter::FunctionArgumentsError();
                } else throw Interpreter::FunctionArgumentsError();
            }
        } else throw Interpreter::FunctionArgumentsError();
    }

    void set_references(Context & context, FunctionContext & function_context, std::map<std::shared_ptr<Expression>, Reference> & computed, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
        if (auto symbol = std::dynamic_pointer_cast<Symbol>(parameters)) {
            auto it = computed.find(arguments);
            auto reference = it != computed.end() ? it->second : (computed[arguments] = Interpreter::execute(context, arguments));
            function_context.add_symbol(symbol->name, reference);
        } else if (auto p_tuple = std::dynamic_pointer_cast<Tuple>(parameters)) {
            if (auto a_tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (p_tuple->objects.size() == a_tuple->objects.size()) {
                    for (size_t i = 0; i < p_tuple->objects.size(); i++)
                        set_references(context, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]);
                } else throw Interpreter::FunctionArgumentsError();
            } else {
                auto it = computed.find(arguments);
                auto reference = it != computed.end() ? it->second : (computed[arguments] = Interpreter::execute(context, arguments));
                set_references(function_context, parameters, reference);
            }
        } else if (auto p_function = std::dynamic_pointer_cast<FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Symbol>(p_function->function)) {
                auto it = computed.find(arguments);
                if (it != computed.end())
                    throw Interpreter::FunctionArgumentsError();

                auto function_definition = std::make_shared<FunctionDefinition>();
                function_definition->parameters = p_function->arguments;
                function_definition->body = arguments;

                auto object = context.new_object();
                object->functions.push_front(CustomFunction{function_definition});
                auto & f = object->functions.back();
                for (auto symbol : function_definition->body->symbols)
                    f.extern_symbols[symbol] = context[symbol];

                function_context.add_symbol(symbol->name, Reference(object));
            } else throw Interpreter::FunctionArgumentsError();
        } else throw Interpreter::FunctionArgumentsError();
    }

    Reference call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, Reference const& reference) {
        Data data = reference.to_data(context);

        for (auto const& function : functions) {
            try {
                FunctionContext function_context(context, nullptr);
                for (auto & symbol : function.extern_symbols)
                    function_context.add_symbol(symbol.first, symbol.second);

                if (auto custom_function = std::get_if<CustomFunction>(&function)) {
                    set_references(function_context, custom_function->pointer->parameters, data);

                    Data filter;
                    if (custom_function->pointer->filter != nullptr)
                        filter = Interpreter::execute(function_context, custom_function->pointer->filter).to_data(context);
                    else filter = nullptr;

                    if (auto b = std::get_if<bool>(&filter)) {
                        if (*b)
                            return Interpreter::execute(function_context, custom_function->pointer->body);
                        else continue;
                    } else FunctionArgumentsError();
                } else if (auto system_function = std::get_if<SystemFunction>(&function)) {
                    set_references(function_context, system_function->parameters, data);

                    return system_function->pointer(function_context);
                } else return Reference();
            } catch (FunctionArgumentsError & e) {}
        }

        if (position != nullptr) {
            position->store_stack_trace(context);
            position->notify_error("Error: cannot find a function matching with the arguments");
        }
        throw Error();
    }

    Reference call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, std::shared_ptr<Expression> arguments) {
        std::map<std::shared_ptr<Expression>, Reference> computed;

        for (auto const& function : functions) {
            try {
                FunctionContext function_context(context, position);
                for (auto & symbol : function.extern_symbols)
                    function_context.add_symbol(symbol.first, symbol.second);

                if (auto custom_function = std::get_if<CustomFunction>(&function)) {
                    set_references(context, function_context, computed, custom_function->pointer->parameters, arguments);

                    Data filter;
                    if (custom_function->pointer->filter != nullptr)
                        filter = execute(function_context, custom_function->pointer->filter).to_data(context);
                    else filter = nullptr;

                    if (auto b = std::get_if<bool>(&filter)) {
                        if (*b)
                            return Interpreter::execute(function_context, custom_function->pointer->body);
                        else continue;
                    } else FunctionArgumentsError();
                } else if (auto system_function = std::get_if<SystemFunction>(&function)) {
                    set_references(context, function_context, computed, system_function->parameters, arguments);

                    return system_function->pointer(function_context);
                } else return Reference();

            } catch (FunctionArgumentsError & e) {}
        }

        if (position != nullptr) {
            position->store_stack_trace(context);
            position->notify_error("Error: cannot find a function matching with the arguments");
        }
        throw Error();
    }


    Reference execute(Context & context, std::shared_ptr<Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
            auto data = execute(context, function_call->function).to_data(context);
            if (auto object = std::get_if<Object*>(&data))
                return call_function(context, function_call->position, (*object)->functions, function_call->arguments);
            else
                return call_function(context, function_call->position, std::list<Function>{}, function_call->arguments);
        } else if (auto function_definition = std::dynamic_pointer_cast<FunctionDefinition>(expression)) {
            auto object = context.new_object();
            object->functions.push_front(CustomFunction{});
            auto & f = object->functions.back();

            for (auto symbol : function_definition->body->symbols)
                if (context.has_symbol(symbol))
                    f.extern_symbols[symbol] = context[symbol];
            if (function_definition->filter != nullptr)
                for (auto symbol : function_definition->filter->symbols)
                    if (context.has_symbol(symbol))
                        f.extern_symbols[symbol] = context[symbol];

            return Reference(object);
        } else if (auto property = std::dynamic_pointer_cast<Property>(expression)) {
            auto data = execute(context, property->object).to_data(context);
            if (auto object = std::get_if<Object*>(&data))
                return PropertyReference{**object, (*object)->get_property(property->name, context)};
            else
                return Reference(context.new_object());
        } else if (auto symbol = std::dynamic_pointer_cast<Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto b = std::get_if<bool>(&data)) {
                return Reference(Data(*b));
            } else if (auto l = std::get_if<long>(&data)) {
                return Reference(Data(*l));
            } else if (auto d = std::get_if<double>(&data)) {
                return Reference(Data(*d));
            } else if (auto str = std::get_if<std::string>(&data)) {
                return Reference(context.new_object(*str));
            } else {
                return context[symbol->name];
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(expression)) {
            if (!tuple->objects.empty()) {
                TupleReference tuple_reference;
                for (auto e : tuple->objects)
                    tuple_reference.push_back(execute(context, e));
                return tuple_reference;
            } else return Reference(context.new_object());
        } else return Reference();
    }


    Reference run(Context & context, std::shared_ptr<Expression> expression) {
        try {
            return Interpreter::execute(context, expression);
        } catch (Base::Exception & ex) {
            ex.position->notify_error("An exception occured");
            return Reference(context.new_object());
        } catch (Error & e) {
            return Reference(context.new_object());
        }
    }

    bool print(std::ostream & stream, Data data) {
        if (auto c = std::get_if<char>(&data)) {
            std::cout << *c;
            return true;
        } else if (auto f = std::get_if<double>(&data)) {
            std::cout << *f;
            return true;
        } else if (auto i = std::get_if<long>(&data)) {
            std::cout << *i;
            return true;
        } else if (auto b = std::get_if<bool>(&data)) {
            std::cout << *b;
            return true;
        } else if (auto object = std::get_if<Object*>(&data)) {
            bool printed = false;
            for (auto d : (*object)->array)
                if (print(stream, d)) printed = true;
            return printed;
        } else return false;
    }

}
