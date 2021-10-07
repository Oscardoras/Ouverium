#ifndef PARSER_EXPRESSION_VARIABLECALL_HPP_
#define PARSER_EXPRESSION_VARIABLECALL_HPP_

#include "Expression.hpp"

class VariableCall: public Expression {

public:

    std::string variableName;
    std::shared_ptr<VariableCall> next = nullptr;

    VariableCall getLastVariable() const {
        if (next == nullptr) return *this;
        else return next->getLastVariable();
    }

    virtual std::string getType() const {
        return "VariableCall";
    }

};


#endif