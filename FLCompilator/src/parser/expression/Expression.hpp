#ifndef PARSER_EXPRESSION_EXPRESSION_HPP_
#define PARSER_EXPRESSION_EXPRESSION_HPP_

#include <string>
#include <memory>
#include <vector>


class Expression {

public:

    std::vector<std::string> usedSymbols;
    std::vector<std::string> newSymbols;

    virtual std::string getType() const {
        return "Expression";
    }

    std::string toString() const;

};


#endif