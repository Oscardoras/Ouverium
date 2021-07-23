#ifndef PARSER_EXPRESSION_DELETION_HPP_
#define PARSER_EXPRESSION_DELETION_HPP_

#include "Expression.hpp"


class Deletion: public Expression {

public:

    std::shared_ptr<Expression> variables;

    virtual std::string getType() const {
        return "Deletion";
    }

};


#endif