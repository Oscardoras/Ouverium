#ifndef PARSER_EXPRESSION_TUPLE_HPP_
#define PARSER_EXPRESSION_TUPLE_HPP_

#include <vector>

#include "Expression.hpp"


class Tuple: public Expression {

public:

    std::vector<std::shared_ptr<Expression>> objects;

    virtual std::string getType() const {
        return "Tuple";
    }

};


#endif