#ifndef PARSER_EXPRESSION_PROPERTY_HPP_
#define PARSER_EXPRESSION_PROPERTY_HPP_

#include "Expression.hpp"

struct Property: public Expression {

    const Type type = Expression::Property;

    std::shared_ptr<Expression> object;
    std::string name;

};


#endif