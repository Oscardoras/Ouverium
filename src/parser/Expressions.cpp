#include "Expressions.hpp"


namespace Parser {

    std::string tabu(int n) {
        std::string s;
        for (int i = 0; i < n; i++) s+= "    ";
        return s;
    }

    std::string expression_to_string(Expression const * const expression, int n) {
        std::string s;

        if (expression == nullptr) return "NULL\n";

        if (auto function_call = dynamic_cast<FunctionCall const * const>(expression)) {
            s += "FunctionCall:\n";
            n++;
            s += tabu(n) + "function: " + expression_to_string(function_call->function.get(), n);
            s += tabu(n) + "object: " + expression_to_string(function_call->arguments.get(), n);
        } else if (auto function_definition = dynamic_cast<FunctionDefinition const * const>(expression)) {
            s += "FunctionDefinition:\n";
            n++;
            s += tabu(n) + "parameters: " + expression_to_string(function_definition->parameters.get(), n);
            s += tabu(n) + "filter: " + expression_to_string(function_definition->filter.get(), n);
            s += tabu(n) + "object: " + expression_to_string(function_definition->body.get(), n);
        } else if (auto property = dynamic_cast<Property const * const>(expression)) {
            s += "Property:\n";
            n++;
            s += tabu(n) + "object: " + expression_to_string(property->object.get(), n);
            s += tabu(n) + "name: " + property->name + "\n";
        } else if (auto symbol = dynamic_cast<Symbol const * const>(expression)) {
            s += "Symbol: " + symbol->name + "\n";
        } else if (auto tuple = dynamic_cast<Tuple const * const>(expression)) {
            s += "Tuple:\n";
            n++;
            for (auto const& ex : tuple->objects) s += tabu(n) + expression_to_string(ex.get(), n);
        }

        return s;
    }

    std::string Expression::to_string() const {
        return expression_to_string(this, 0);
    }

}
