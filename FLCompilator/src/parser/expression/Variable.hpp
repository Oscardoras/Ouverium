#ifndef PARSER_EXPRESSION_VARIABLE_HPP_
#define PARSER_EXPRESSION_VARIABLE_HPP_

#include "Expression.hpp"

class Variable: public Expression {

public:

    std::string variableName;
    std::shared_ptr<Variable> next = nullptr;

    Variable getLastVariable() const {
        if (next == nullptr) return *this;
        else return next->getLastVariable();
    }

    virtual std::string getType() const {
        return "VariableCall";
    }

};


#endif