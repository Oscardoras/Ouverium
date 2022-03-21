#ifndef PARSER_EXPRESSION_TUPLE_HPP_
#define PARSER_EXPRESSION_TUPLE_HPP_

#include <vector>

#include "Expression.hpp"


struct Tuple: public Expression {

    const Type type = Expression::Tuple;

    std::vector<std::shared_ptr<Expression>> objects;

};


#endif