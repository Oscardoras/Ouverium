#ifndef PARSER_EXPRESSION_FUNCTIONCALL_HPP_
#define PARSER_EXPRESSION_FUNCTIONCALL_HPP_

#include "Expression.hpp"


struct FunctionCall: public Expression {

    FunctionCall() {
        type = Type::FunctionCall;
    }

    std::shared_ptr<Expression> function;
    std::shared_ptr<Expression> object;

};


#endif