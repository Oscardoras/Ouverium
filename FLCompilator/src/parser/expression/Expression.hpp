#ifndef PARSER_EXPRESSION_EXPRESSION_HPP_
#define PARSER_EXPRESSION_EXPRESSION_HPP_

#include <string>
#include <memory>


class Expression {

public:

    virtual std::string getType() const = 0;

};


#endif