#ifndef __PARSER_EXPRESSION_PROPERTY_HPP__
#define __PARSER_EXPRESSION_PROPERTY_HPP__

#include "Expression.hpp"

struct Property: public Expression {

    Property() {
        type = Type::Property;
    }

    std::unique_ptr<Expression> object;
    std::string name;

};


#endif
