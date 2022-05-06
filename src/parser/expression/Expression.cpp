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

std::string expressionToString(Expression const * const expression, int n) {
    std::string s = "";

    if (expression == nullptr) return "NULL\n";

    if (n == 0) {
        s += "globalSymbols :\n";
        for (std::string name : expression->usedSymbols) s += tabu(1) + " " + name + "\n";
    }

    if (expression->type == Expression::FunctionCall) {
        FunctionCall * functionCall = (FunctionCall *) expression;

        s += "FunctionCall :\n";
        n++;
        s += tabu(n) + "function : " + expressionToString(functionCall->function.get(), n);
        s += tabu(n) + "object : " + expressionToString(functionCall->object.get(), n);
    } else if (expression->type == Expression::FunctionDefinition) {
        FunctionDefinition * functionDefinition = (FunctionDefinition *) expression;

        s += "FunctionDefinition :\n";
        n++;
        s += tabu(n) + "parameters : " + expressionToString(functionDefinition->parameters.get(), n);
        s += tabu(n) + "filter : " + expressionToString(functionDefinition->filter.get(), n);
        s += tabu(n) + "object : " + expressionToString(functionDefinition->object.get(), n);
        s += tabu(n) + "localSymbols :\n";
        n++;
        for (std::string name : functionDefinition->object->usedSymbols) s += tabu(n) + " " + name + "\n";
    } else if (expression->type == Expression::Property) {
        Property * property = (Property *) expression;
        
        s += "Property :\n";
        n++;
        s += tabu(n) + "object : " + expressionToString(property->object.get(), n);
        s += tabu(n) + "name : " + property->name + "\n";
    } else if (expression->type == Expression::Symbol) {
        Symbol * symbol = (Symbol *) expression;
        
        s += "Symbol : " + symbol->name + "\n";
    } else if (expression->type == Expression::Tuple) {
        Tuple * tuple = (Tuple *) expression;

        s += "Tuple :\n";
        n++;
        for (std::shared_ptr<Expression> ex : tuple->objects) s += tabu(n) + expressionToString(ex.get(), n);
    }

    if (!expression->newSymbols.empty()) {
        s += tabu(n-1) + "newSymbols :\n";
        for (std::string name : expression->newSymbols) s += tabu(n) + " " + name + "\n";
    }

    return s;
}

Expression::Expression() {
    escaped = false;
}

std::string Expression::toString() const {
    return expressionToString(this, 0);
}