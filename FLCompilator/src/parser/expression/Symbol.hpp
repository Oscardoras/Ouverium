#ifndef PARSER_EXPRESSION_SYMBOL_HPP_
#define PARSER_EXPRESSION_SYMBOL_HPP_

#include "Expression.hpp"

struct Symbol: public Expression {

    const Type type = Expression::Symbol;

    std::string name;

};


#endif