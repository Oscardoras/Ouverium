#ifndef PARSER_EXPRESSION_PROPERTYCALL_HPP_
#define PARSER_EXPRESSION_PROPERTYCALL_HPP_

#include "Expression.hpp"

class PropertyCall: public Expression {

public:

    std::shared_ptr<Expression> object;
    std::string variableName;

    virtual std::string getType() const {
        return "PropertyCall";
    }

};


#endif