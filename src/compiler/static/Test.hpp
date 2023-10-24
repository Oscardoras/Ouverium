#pragma once

#include "Reference.hpp"


namespace Test {

    struct Call {
        std::shared_ptr<Reference> function;
        std::shared_ptr<Reference> in;
        std::shared_ptr<ForwardedReference> out;
    };

    struct Context {
        std::map<std::string, std::shared_ptr<Reference>> symbols;
        std::shared_ptr<Reference> out;
    };

    struct Analysis {

        std::map<std::shared_ptr<Parser::FunctionCall>, Call> calls;
        std::map<std::shared_ptr<Parser::FunctionDefinition>, Context> contexts;

    };

    std::shared_ptr<Reference> execute(Analysis & analysis, Context & context, std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            Call call;

            call.function = execute(analysis, context, function_call->function);
            call.in = execute(analysis, context, function_call->arguments);
            call.out = std::make_shared<ForwardedReference>();

            analysis.calls[function_call] = call;

            return call.out;
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            execute_function(analysis, context, function_definition);

            CustomFunction function {
                .function = function_definition
            };

            return std::make_shared<DirectReference>(Data{ .function = function });
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            return std::make_shared<PropertyReference>(PropertyReference{
                execute(analysis, context, property->object),
                property->name
            });
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto b = std::get_if<bool>(&data)) {
                return std::make_shared<DirectReference>(*b);
            } else if (auto l = std::get_if<long>(&data)) {
                return std::make_shared<DirectReference>(*l);
            } else if (auto d = std::get_if<double>(&data)) {
                return std::make_shared<DirectReference>(*d);
            } else if (auto str = std::get_if<std::string>(&data)) {
                // TODO
            } else {
                return context.symbols[symbol->name];
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
            auto tuple_reference = std::make_shared<TupleReference>();
            for (auto e : tuple->objects)
                tuple_reference->push_back(execute(analysis, context, e));
            return tuple_reference;
        } else return nullptr;
    }

    void get_parameters(std::set<std::string> & symbols, std::shared_ptr<Parser::Expression> parameters) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            symbols.insert(symbol->name);
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            for (auto const& e : p_tuple->objects)
                get_parameters(symbols, e);
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            // TODO
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            get_parameters(symbols, p_property->object);
        }
    }

    void execute_function(Analysis & analysis, Context & context, std::shared_ptr<Parser::FunctionDefinition> const& function_definition) {
        auto & function_context = analysis.contexts[function_definition];

        std::set<std::string> symbols;

        for (auto symbol : function_definition->symbols) {
            auto it = context.symbols.find(symbol);
            if (it != context.symbols.end())
                function_context.symbols[symbol] = it->second;
        }

        get_parameters(symbols, function_definition->parameters);
        for (auto const& symbol : symbols)
            function_context.symbols[symbol] = std::make_shared<ForwardedReference>();

        if (function_definition->filter != nullptr)
            execute(analysis, function_context, function_definition->filter);

        function_context.out = execute(analysis, function_context, function_definition->body);
    }

    class FunctionArgumentsError {};

    void set_arguments(Analysis & analysis, Context & context, Context & function_context, std::shared_ptr<Parser::Expression> parameters, std::shared_ptr<Reference> argument) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            if (function_context.symbols.find(symbol->name) == function_context.symbols.end()) {
                std::static_pointer_cast<ForwardedReference>(function_context.symbols[symbol->name])->link(argument);
            }
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto tuple_reference = std::dynamic_pointer_cast<TupleReference>(argument)) {
                if (tuple_reference->size() == p_tuple->objects.size()) {
                    for (size_t i = 0; i < p_tuple->objects.size(); i++)
                        set_arguments(analysis, context, function_context, p_tuple->objects[i], (*tuple_reference)[i]);
                } else throw FunctionArgumentsError();
            } else {
                // TODO
            }
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(p_function->function)) {
                if (function_context.symbols.find(symbol->name) == function_context.symbols.end()) {
                    auto lambda = std::make_shared<DirectReference>(Data{ .function = LambdaFunction{ argument } });
                    std::static_pointer_cast<ForwardedReference>(function_context.symbols[symbol->name])->link(lambda);
                    return;
                }
            }

            auto r = execute(context, p_function->function).to_data(context);
            std::list<Function> functions;
            try {
                functions = r.get<Object*>()->functions;
            } catch (Data::BadAccess const& e) {}

            Arguments args;
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                auto it = computed.find(*expression);
                args = it != computed.end() ? it->second : arguments;
            } else {
                args = arguments;
            }

            Reference reference;
            try {
                reference = call_function(context, p_function, functions, args);
            } catch (Exception const& e) {
                throw Interpreter::FunctionArgumentsError();
            }
            set_arguments(context, function_context, computed, p_function->arguments, reference);
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (auto property_reference = std::get_if<PropertyReference>(&reference)) {
                if (p_property->name == property_reference->name || p_property->name == ".") {
                    set_arguments(context, function_context, computed, p_property->object, Reference(Data(&property_reference->parent.get())));
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
        } else throw Interpreter::FunctionArgumentsError();
    }

    void call_function(Analysis & analysis, std::shared_ptr<Parser::FunctionCall> call, Function & function) {

    }

    void link(Analysis const& analysis) {
        for (auto const& [function, call] : analysis.calls) {
            for (auto f : call.function->get_functions()) {
                f
            }
        }
    }

}
