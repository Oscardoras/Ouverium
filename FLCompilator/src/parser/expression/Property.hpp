#ifndef PARSER_EXPRESSION_PROPERTY_HPP_
#define PARSER_EXPRESSION_PROPERTY_HPP_

#include "Expression.hpp"

class Property: public Expression {

public:

    std::shared_ptr<Expression> object;
    std::string name;

    virtual std::string getType() const {
        return "Property";
    }

};


#endif