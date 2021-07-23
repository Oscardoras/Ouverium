#ifndef PARSER_EXPRESSION_ALTERNATIVECONDITION_HPP_
#define PARSER_EXPRESSION_ALTERNATIVECONDITION_HPP_

#include "Condition.hpp"


class AlternativeCondition: public Condition {

public:

    std::shared_ptr<Expression> alternative;

    virtual std::string getType() const {
        return "AlternativeCondition";
    }

};


#endif