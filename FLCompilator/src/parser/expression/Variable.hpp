#ifndef PARSER_EXPRESSION_VARIABLE_HPP_
#define PARSER_EXPRESSION_VARIABLE_HPP_

#include "Expression.hpp"

class Variable: public Expression {

public:

    std::string variableName;
    
    virtual std::string getType() const {
        return "Variable";
    }

};


#endif