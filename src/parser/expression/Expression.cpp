#include "FunctionCall.hpp"
#include "FunctionDefinition.hpp"
#include "Property.hpp"
#include "Symbol.hpp"
#include "Tuple.hpp"


std::string tabu(int n) {
    std::string s;
    for (int i = 0; i < n; i++) s+= "    ";
    return s;
}

std::string expression_to_string(Expression const * const expression, int n) {
    std::string s;

    if (expression == nullptr) return "NULL\n";

    if (expression->type == Expression::FunctionCall) {
        auto functionCall = (FunctionCall*) expression;

        s += "FunctionCall:\n";
        n++;
        s += tabu(n) + "function: " + expression_to_string(functionCall->function.get(), n);
        s += tabu(n) + "object: " + expression_to_string(functionCall->object.get(), n);
    } else if (expression->type == Expression::FunctionDefinition) {
        auto functionDefinition = (FunctionDefinition*) expression;

        s += "FunctionDefinition:\n";
        n++;
        s += tabu(n) + "parameters: " + expression_to_string(functionDefinition->parameters.get(), n);
        s += tabu(n) + "filter: " + expression_to_string(functionDefinition->filter.get(), n);
        s += tabu(n) + "object: " + expression_to_string(functionDefinition->object.get(), n);
    } else if (expression->type == Expression::Property) {
        auto property = (Property*) expression;

        s += "Property:\n";
        n++;
        s += tabu(n) + "object: " + expression_to_string(property->object.get(), n);
        s += tabu(n) + "name: " + property->name + "\n";
    } else if (expression->type == Expression::Symbol) {
        auto symbol = (Symbol*) expression;

        s += "Symbol: " + symbol->name + "\n";
    } else if (expression->type == Expression::Tuple) {
        auto tuple = (Tuple*) expression;

        s += "Tuple:\n";
        n++;
        for (auto const& ex : tuple->objects) s += tabu(n) + expression_to_string(ex.get(), n);
    }

    return s;
}

std::string Expression::to_string() const {
    return expression_to_string(this, 0);
}
