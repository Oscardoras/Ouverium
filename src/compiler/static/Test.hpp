#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <variant>

#include "../../parser/Parser.hpp"

#include "../../Utils.hpp"


namespace Test {

    struct Function;
    struct Data {
        std::optional<bool> Bool;
        std::optional<char> Char;
        std::optional<long> Int;
        std::optional<double> Float;

        std::optional<Function> function;
    };

    struct LambdaFunction {
        std::shared_ptr<Reference> value;
    };
    struct CustomFunction {
        std::shared_ptr<Parser::FunctionDefinition> function;
        std::vector<Reference> captures;
    };
    struct SystemFunction {};
    struct Function : public std::variant<LambdaFunction, CustomFunction, SystemFunction> {
        using std::variant<LambdaFunction, CustomFunction, SystemFunction>::variant;
    };

    struct Reference {
    protected:
        std::set<std::weak_ptr<ForwardedReference>> children;
    public:

        virtual Data& get_data() = 0;

        virtual std::vector<Function> const& get_functions() {
            std::vector<Function> functions;

            Data data = get_data();
            if (data.function)
                functions.push_back(*data.function);

            for (auto const& child : children)
                for (auto const& f : child.lock()->get_functions())
                    functions.push_back(f);

            return functions;
        }
    };
    struct ForwardedReference : public Reference {
    protected:
        std::set<std::weak_ptr<ForwardedReference>> parents;
    public:
        std::vector<Function> const& get_functions() override {
            return
        }
    };
    struct DirectReference : public Reference, public Data {
        using Data::Data;
        Data& get_data() override {
            return *this;
        }
    };
    struct SymbolReference : public Reference {
        std::shared_ptr<Data> data;
        Data& get_data() override {
            return *data;
        }
    };
    struct PropertyReference : public Reference {
        std::shared_ptr<Reference> parent;
        std::string name;
        Data& get_data() override {
            // TODO
        }
    };
    struct ArrayReference : public Reference {
        std::shared_ptr<Reference> array;
        Data& get_data() override {
            // TODO
        }
    };
    struct TupleReference : public Reference, public std::vector<std::shared_ptr<Reference>> {
        using std::vector<std::shared_ptr<Reference>>::vector;
        Data& get_data() override {
            // TODO
        }
    };

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

    std::shared_ptr<Reference> execute(Analysis & analysis, std::shared_ptr<Parser::FunctionDefinition> const& context, std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            Call call;

            call.function = execute(analysis, context, function_call->function);
            call.in = execute(analysis, context, function_call->arguments);
            call.out = std::make_shared<ForwardedReference>();

            analysis.calls[function_call] = call;

            return call.out;
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            execute_function(analysis, function_definition);

            CustomFunction function {
                .function = function_definition
            };
            // TODO : captures

            return std::make_shared<DirectReference>(Data{ .function = function });
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            return std::make_shared<PropertyReference>(PropertyReference{
                .parent = execute(analysis, context, property->object),
                .name = property->name
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
                return analysis.contexts[context].symbols[symbol->name];
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

    void execute_function(Analysis & analysis, std::shared_ptr<Parser::FunctionDefinition> const& function) {
        std::set<std::string> symbols;
        get_parameters(symbols, function->parameters);
        for (auto const& symbol : symbols)
            analysis.contexts[function].symbols[symbol] = std::make_shared<ForwardedReference>();

        if (function->filter != nullptr)
            execute(analysis, function, function->filter);

        analysis.contexts[function].out = execute(analysis, function, function->body);
    }

    void call_function(Analysis & analysis, std::shared_ptr<Parser::FunctionCall> call, Function & function) {

    }

    void link(Analysis const& analysis) {
        for (auto const& [function, call] : analysis.calls) {
            call.
        }
    }

}
