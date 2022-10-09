#ifndef __PARSER_EXPRESSION_EXPRESSION_HPP__
#define __PARSER_EXPRESSION_EXPRESSION_HPP__

#include <string>
#include <memory>
#include <vector>

#include "../Position.hpp"


/**
 * Represents an expression of the language, must be inherited.
*/
struct Expression {

    enum Type {
        FunctionCall,
        FunctionDefinition,
        Property,
        Symbol,
        Tuple
    } type;

    /**
     * The position of the expression in the source.
    */
    std::shared_ptr<Position> position;

    /**
     * The list of the symbols available in the expression.
    */
    std::vector<std::string> symbols;

    /**
     * Gets a string of the expression to print it as a tree.
     * @return the expression as a string.
    */
    std::string to_string() const;

};


#endif
