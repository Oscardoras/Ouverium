#ifndef PARSER_EXPRESSION_EXPRESSION_HPP_
#define PARSER_EXPRESSION_EXPRESSION_HPP_

#include <string>
#include <memory>
#include <vector>

#include "../Position.hpp"


struct Expression {

    enum Type {
        FunctionCall,
        FunctionDefinition,
        Property,
        Symbol,
        Tuple
    };

    Type type;

    std::shared_ptr<Position> position;

    bool escaped;
    std::vector<std::string> usedSymbols;
    std::vector<std::string> newSymbols;

    Expression();

    std::string toString() const;

};


#endif