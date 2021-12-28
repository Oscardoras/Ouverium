#ifndef PARSER_EXPRESSION_SYMBOL_HPP_
#define PARSER_EXPRESSION_SYMBOL_HPP_

#include "Expression.hpp"

class Symbol: public Expression {

public:

    std::string name;
    
    virtual std::string getType() const {
        return "Symbol";
    }

};


#endif