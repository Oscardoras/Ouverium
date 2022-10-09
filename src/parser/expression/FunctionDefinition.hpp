#ifndef __PARSER_EXPRESSION_FUNCTIONDEFINITION_HPP__
#define __PARSER_EXPRESSION_FUNCTIONDEFINITION_HPP__

#include <vector>

#include "Expression.hpp"


struct FunctionDefinition: public Expression {

    FunctionDefinition() {
        type = Type::FunctionDefinition;
    }

    std::unique_ptr<Expression> parameters;
    std::unique_ptr<Expression> filter;
    std::unique_ptr<Expression> object;

};


#endif
