#ifndef PARSER_EXPRESSION_SYMBOL_HPP_
#define PARSER_EXPRESSION_SYMBOL_HPP_

#include "Expression.hpp"

struct Symbol: public Expression {

    Symbol() {
        type = Type::Symbol;
    }

    std::string name;

};


#endif