#ifndef PARSER_EXPRESSION_CONDITIONALTERNATIVE_HPP_
#define PARSER_EXPRESSION_CONDITIONALTERNATIVE_HPP_

#include "Condition.hpp"


class ConditionAlternative: public Condition {

public:

    std::shared_ptr<Expression> alternative;

    virtual std::string getType() const {
        return "ConditionAlternative";
    }

};


#endif