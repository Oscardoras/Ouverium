#ifndef PARSER_EXPRESSION_FUNCTIONDEFINITION_HPP_
#define PARSER_EXPRESSION_FUNCTIONDEFINITION_HPP_

#include <vector>

#include "Expression.hpp"


struct FunctionDefinition: public Expression {

    const Type type = Expression::FunctionDefinition;

    std::shared_ptr<Expression> parameters;
    std::shared_ptr<Expression> filter;
    std::shared_ptr<Expression> object;

};


#endif