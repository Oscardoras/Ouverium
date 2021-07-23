#ifndef PARSER_EXPRESSION_CONDITION_HPP_
#define PARSER_EXPRESSION_CONDITION_HPP_

#include "Expression.hpp"


class Condition: public Expression {

public:

    std::shared_ptr<Expression> condition;
    std::shared_ptr<Expression> object;

    virtual std::string getType() const {
        return "Condition";
    }

};


#endif