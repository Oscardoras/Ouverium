#ifndef __PARSER_EXPRESSION_EXPRESSION_HPP__
#define __PARSER_EXPRESSION_EXPRESSION_HPP__

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
    } type;

    std::shared_ptr<Position> position;
    std::vector<std::string> symbols;

    std::string to_string() const;

};


#endif
