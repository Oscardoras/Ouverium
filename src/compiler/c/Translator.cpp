#include <algorithm>
#include <map>

#include "Translator.hpp"


namespace CTranslator {

    void iterate(std::shared_ptr<Expression> const& expression, std::vector<Function> & functions, std::vector<std::string> defined_symbols) {
        if (expression->type == Expression::FunctionDefinition) {
            auto function_definition = std::static_pointer_cast<FunctionDefinition>(expression);

            functions.push_back(Function {});
            auto & function = functions.back();

            function.expression = function_definition;

            for (auto const& s : function_definition->parameters->symbols)
                if (std::find(defined_symbols.begin(), defined_symbols.end(), s) == defined_symbols.end()) {
                    function.parameters.push_back(Symbol{ .name = s });
                    defined_symbols.push_back(s);
                }

            if (function_definition->filter != nullptr)
                for (auto const& s : function_definition->filter->symbols)
                    if (std::find(defined_symbols.begin(), defined_symbols.end(), s) == defined_symbols.end()) {
                        function.local_variables.push_back(Symbol{ .name = s });
                        defined_symbols.push_back(s);
                    }

            for (auto const& s : function_definition->object->symbols)
                if (std::find(defined_symbols.begin(), defined_symbols.end(), s) == defined_symbols.end()) {
                    function.local_variables.push_back(Symbol{ .name = s });
                    defined_symbols.push_back(s);
                }
        } else if (expression->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(expression);

            iterate(function_call->function, functions, defined_symbols);
            iterate(function_call->object, functions, defined_symbols);
        } else if (expression->type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            iterate(property->object, functions, defined_symbols);
        } else if (expression->type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            iterate(property->object, functions, defined_symbols);
        } else if (expression->type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(expression);

            for (auto const& o : tuple->objects) {
                iterate(o, functions, defined_symbols);
            }
        }
    }

    Environment get_environment(std::shared_ptr<Expression> tree) {
        Environment env;

        for (auto const& s : tree->symbols) {
            env.global_variables.push_back(Symbol{ .name = s });
        }

        auto defined_symbols = tree->symbols;
        iterate(tree, env.functions, defined_symbols);

        return env;
    }

    void execute(Function & function, Environment & environment) {

    }

    void execute(std::shared_ptr<Expression> tree, Environment & environment, std::vector<Object> & objects) {
        if (tree->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(tree);


        } else if (tree->type == Expression::FunctionDefinition) {
            auto function_definition = std::static_pointer_cast<FunctionDefinition>(tree);

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
        }
    }

}
