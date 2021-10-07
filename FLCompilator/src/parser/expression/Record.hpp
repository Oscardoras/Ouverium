#ifndef PARSER_EXPRESSION_RECORD_HPP_
#define PARSER_EXPRESSION_RECORD_HPP_

#include <vector>

#include "Expression.hpp"


struct Field {
    std::string name;
    std::shared_ptr<Field> object;
};

class Record: public Expression {

public:

    std::vector<Field> fields;

    virtual std::string getType() const {
        return "Record";
    }

};


#endif