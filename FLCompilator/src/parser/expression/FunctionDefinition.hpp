#ifndef PARSER_EXPRESSION_FUNCTIONDEFINITION_HPP_
#define PARSER_EXPRESSION_FUNCTIONDEFINITION_HPP_

#include <vector>

#include "Expression.hpp"


class FunctionDefinition: public Expression {

public:

    std::shared_ptr<Expression> parameters;
    std::shared_ptr<Expression> filter;
    std::shared_ptr<Expression> object;

    virtual std::string getType() const {
        return "FunctionDefinition";
    }

};


#endif