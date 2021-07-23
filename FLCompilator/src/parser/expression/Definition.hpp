#ifndef PARSER_EXPRESSION_DEFINITION_HPP_
#define PARSER_EXPRESSION_DEFINITION_HPP_

#include "Expression.hpp"


class Definition: public Expression {

public:

    bool implicit;
    std::shared_ptr<Expression> variables;
    std::shared_ptr<Expression> object;

    virtual std::string getType() const {
        return "Definition";
    }

};


#endif