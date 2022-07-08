#ifndef __PARSER_EXPRESSION_SYMBOL_HPP__
#define __PARSER_EXPRESSION_SYMBOL_HPP__

#include "Expression.hpp"

struct Symbol: public Expression {

    Symbol() {
        type = Type::Symbol;
    }

    std::string name;

};


#endif
