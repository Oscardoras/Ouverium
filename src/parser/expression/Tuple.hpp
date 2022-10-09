#ifndef __PARSER_EXPRESSION_TUPLE_HPP__
#define __PARSER_EXPRESSION_TUPLE_HPP__

#include <vector>

#include "Expression.hpp"


struct Tuple: public Expression {

    Tuple() {
        type = Type::Tuple;
    }

    std::vector<std::unique_ptr<Expression>> objects;

};


#endif
