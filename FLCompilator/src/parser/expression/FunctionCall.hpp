#ifndef PARSER_EXPRESSION_FUNCTIONCALL_HPP_
#define PARSER_EXPRESSION_FUNCTIONCALL_HPP_

#include "Expression.hpp"


class FunctionCall: public Expression {

public:

    std::shared_ptr<Expression> function;
    std::shared_ptr<Expression> object;

    virtual std::string getType() const {
        return "FunctionCall";
    }

};


#endif