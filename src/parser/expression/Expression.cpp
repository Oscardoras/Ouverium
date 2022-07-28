#include "FunctionCall.hpp"
#include "FunctionDefinition.hpp"
#include "Property.hpp"
#include "Symbol.hpp"
#include "Tuple.hpp"


std::string tabu(int n) {
    std::string s = "";
    for (int i = 0; i < n; i++) s+= "    ";
    return s;
}

std::string expression_to_string(Expression const * const expression, int n) {
    std::string s = "";

    if (expression == nullptr) return "NULL\n";

    /*
    if (n == 0) {
        s += "global symbols :\n";
        for (std::string name : expression->symbols)
            s += tabu(1) + name + "\n";
    }
    */

    if (expression->type == Expression::FunctionCall) {
        FunctionCall * functionCall = (FunctionCall *) expression;

        s += "FunctionCall :\n";
        n++;
        s += tabu(n) + "function : " + expression_to_string(functionCall->function.get(), n);
        s += tabu(n) + "object : " + expression_to_string(functionCall->object.get(), n);
    } else if (expression->type == Expression::FunctionDefinition) {
        FunctionDefinition * functionDefinition = (FunctionDefinition *) expression;

        s += "FunctionDefinition :\n";
        n++;
        s += tabu(n) + "parameters : " + expression_to_string(functionDefinition->parameters.get(), n);
        s += tabu(n) + "filter : " + expression_to_string(functionDefinition->filter.get(), n);
        s += tabu(n) + "object : " + expression_to_string(functionDefinition->object.get(), n);
        /*
        s += tabu(n) + "symbols :\n";
        n++;
        for (std::string name : functionDefinition->object->symbols)
           *s += tabu(n) + name + "\n";
        */
    } else if (expression->type == Expression::Property) {
        Property * property = (Property *) expression;
        
        s += "Property :\n";
        n++;
        s += tabu(n) + "object : " + expression_to_string(property->object.get(), n);
        s += tabu(n) + "name : " + property->name + "\n";
    } else if (expression->type == Expression::Symbol) {
        Symbol * symbol = (Symbol *) expression;
        
        s += "Symbol : " + symbol->name + "\n";
    } else if (expression->type == Expression::Tuple) {
        Tuple * tuple = (Tuple *) expression;

        s += "Tuple :\n";
        n++;
        for (std::shared_ptr<Expression> ex : tuple->objects) s += tabu(n) + expression_to_string(ex.get(), n);
    }

    return s;
}

std::string Expression::to_string() const {
    return expression_to_string(this, 0);
}
