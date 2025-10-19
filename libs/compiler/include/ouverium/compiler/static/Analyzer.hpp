#pragma once

#include "Reference.hpp"


namespace Static {

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

        std::set<std::pair<std::shared_ptr<Parser::FunctionCall>, Function>> couples;

    };

    std::shared_ptr<Reference> execute(Analysis& analysis, Context& context, std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            Call call;

            call.function = execute(analysis, context, function_call->function);
            call.in = execute(analysis, context, function_call->arguments);
            call.out = std::make_shared<ForwardedReference>();

            analysis.calls[function_call] = call;

            return call.out;
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            execute_function(analysis, context, function_definition);

            CustomFunction function{
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
            } else if (auto l = std::get_if<OV_INT>(&data)) {
                return std::make_shared<DirectReference>(*l);
            } else if (auto d = std::get_if<OV_FLOAT>(&data)) {
                return std::make_shared<DirectReference>(*d);
            } else if (auto str = std::get_if<std::string>(&data)) {
                std::vector<std::shared_ptr<Reference>> string;
                for (auto c : *str)
                    string.push_back(std::make_shared<DirectReference>(Data{ .Char = c }));
                return std::make_shared<TupleReference>(string);
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

    void get_parameters(Analysis& analysis, Context& context, std::set<std::string>& symbols, std::shared_ptr<Parser::Expression> parameters) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            symbols.insert(symbol->name);
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            for (auto const& e : tuple->objects)
                get_parameters(analysis, context, symbols, e);
        } else if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(function_call->function)) {
                if (context.symbols.find(symbol->name) == context.symbols.end()) {
                    symbols.insert(symbol->name);
                    return;
                }
            }

            Call call;

            call.function = execute(analysis, context, function_call->function);
            call.in = std::make_shared<ForwardedReference>();
            call.out = std::make_shared<ForwardedReference>();
            // TODO

            analysis.calls[function_call] = call;

            get_parameters(analysis, context, symbols, function_call->arguments);
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            get_parameters(analysis, context, symbols, property->object);
        }
    }

    void execute_function(Analysis& analysis, Context& context, std::shared_ptr<Parser::FunctionDefinition> const& function_definition) {
        auto& function_context = analysis.contexts[function_definition];

        std::set<std::string> symbols;

        for (auto symbol : function_definition->symbols) {
            auto it = context.symbols.find(symbol);
            if (it != context.symbols.end())
                function_context.symbols[symbol] = it->second;
            else
                function_context.symbols[symbol] = std::make_shared<SymbolReference>();
        }

        get_parameters(analysis, function_context, symbols, function_definition->parameters);
        for (auto const& symbol : symbols)
            function_context.symbols[symbol] = std::make_shared<ForwardedReference>();

        if (function_definition->filter != nullptr)
            execute(analysis, function_context, function_definition->filter);

        function_context.out = execute(analysis, function_context, function_definition->body);
    }

    class FunctionArgumentsError {};

    void set_arguments(Analysis& analysis, Context& function_context, std::shared_ptr<Parser::Expression> parameters, std::shared_ptr<Reference> argument) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            if (function_context.symbols.find(symbol->name) == function_context.symbols.end()) {
                std::static_pointer_cast<ForwardedReference>(function_context.symbols[symbol->name])->link(argument);
            }
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto tuple_reference = std::dynamic_pointer_cast<TupleReference>(argument)) {
                if (tuple_reference->size() == p_tuple->objects.size()) {
                    for (size_t i = 0; i < p_tuple->objects.size(); ++i)
                        set_arguments(analysis, function_context, p_tuple->objects[i], (*tuple_reference)[i]);
                } else throw FunctionArgumentsError();
            } else {
                for (size_t i = 0; i < p_tuple->objects.size(); ++i)
                    set_arguments(analysis, function_context, p_tuple->objects[i], std::make_shared<ArrayReference>(argument, 0));
            }
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(p_function->function)) {
                if (function_context.symbols.find(symbol->name) == function_context.symbols.end()) {
                    auto lambda = std::make_shared<DirectReference>(Data{ .function = LambdaFunction{ argument } });
                    std::static_pointer_cast<ForwardedReference>(function_context.symbols[symbol->name])->link(lambda);
                    return;
                }
            }

            // TODO
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            if (auto property_reference = std::dynamic_pointer_cast<PropertyReference>(argument)) {
                if (p_property->name == property_reference->name || p_property->name == ".") {
                    set_arguments(analysis, function_context, p_property->object, property_reference->parent);
                } else throw FunctionArgumentsError();
            } else throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    void call_function(Analysis& analysis, std::shared_ptr<Parser::FunctionCall> function_call, Function const& function) {
        auto& call = analysis.calls[function_call];

        if (auto custom = std::get_if<CustomFunction>(&function)) {
            auto& context = analysis.contexts[custom->function];

            set_arguments(analysis, context, custom->function->parameters, call.in);
            call.out->link(context.out);
        } else if (auto system = std::get_if<SystemFunction>(&function)) {
            auto r = (*system)(call.in);
            if (r)
                call.out->link(r);
        }
    }

    void link(Analysis& analysis) {
        size_t size = 0;
        do {
            size = analysis.couples.size();

            for (auto const& [function, call] : analysis.calls) {
                for (auto const& f : call.function->get_functions()) {
                    call_function(analysis, function, f);
                    analysis.couples.insert({ function, f });
                }
            }
        } while (size < analysis.couples.size());
    }

}
