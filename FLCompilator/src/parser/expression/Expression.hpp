#ifndef PARSER_EXPRESSION_EXPRESSION_HPP_
#define PARSER_EXPRESSION_EXPRESSION_HPP_

#include <string>
#include <memory>
#include <vector>


struct Expression {

    enum Type {
        FunctionCall,
        FunctionDefinition,
        Property,
        Symbol,
        Tuple
    };

    const Type type;

    std::vector<std::string> usedSymbols;
    std::vector<std::string> newSymbols;

    std::string toString() const;

};


#endif