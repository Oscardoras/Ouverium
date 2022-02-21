#include "Condition.hpp"
#include "ConditionAlternative.hpp"
#include "ConditionRepeat.hpp"
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

    std::string type = expression->getType();
    if (type == "Condition") {
        Condition * condition = (Condition *) expression;

        s += "Condition :\n";
        n++;
        s += tabu(n) + "condition : " + expressionToString(condition->condition.get(), n);
        s += tabu(n) + "object : " + expressionToString(condition->object.get(), n);
    } else if (type == "ConditionAlternative") {
        ConditionAlternative * alternativeCondition = (ConditionAlternative *) expression;

        s += "ConditionAlternative :\n";
        n++;
        s += tabu(n) + "condition : " + expressionToString(alternativeCondition->condition.get(), n);
        s += tabu(n) + "object : " + expressionToString(alternativeCondition->object.get(), n);
        s += tabu(n) + "alternative : " + expressionToString(alternativeCondition->alternative.get(), n);
    } else if (type == "ConditionRepeat") {
        ConditionRepeat * conditionRepeat = (ConditionRepeat *) expression;

        s += "ConditionRepeat :\n";
        n++;
        s += tabu(n) + "condition : " + expressionToString(conditionRepeat->condition.get(), n);
        s += tabu(n) + "object : " + expressionToString(conditionRepeat->object.get(), n);
    } else if (type == "FunctionCall") {
        FunctionCall * functionCall = (FunctionCall *) expression;

        s += "FunctionCall :\n";
        n++;
        s += tabu(n) + "function : " + expressionToString(functionCall->function.get(), n);
        s += tabu(n) + "object : " + expressionToString(functionCall->object.get(), n);
    } else if (type == "FunctionDefinition") {
        FunctionDefinition * functionDefinition = (FunctionDefinition *) expression;

        s += "FunctionDefinition :\n";
        n++;
        s += tabu(n) + "parameters : " + expressionToString(functionDefinition->parameters.get(), n);
        s += tabu(n) + "filter : " + expressionToString(functionDefinition->filter.get(), n);
        s += tabu(n) + "object : " + expressionToString(functionDefinition->object.get(), n);
        s += tabu(n) + "localSymbols :\n";
        n++;
        for (std::string name : functionDefinition->object->usedSymbols) s += tabu(n) + " " + name + "\n";
    } else if (type == "Property") {
        Property * property = (Property *) expression;
        
        s += "Property :\n";
        n++;
        s += tabu(n) + "object : " + expressionToString(property->object.get(), n);
        s += tabu(n) + "name : " + property->name + "\n";
    } else if (type == "Symbol") {
        Symbol * symbol = (Symbol *) expression;
        
        s += "Symbol : " + symbol->name + "\n";
    } else if (type == "Tuple") {
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

std::string Expression::toString() const {
    return expressionToString(this, 0);
}