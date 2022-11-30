#ifndef __EXPRESSIONS_HPP__
#define __EXPRESSIONS_HPP__

#include <string>
#include <memory>
#include <vector>

#include "parser/Position.hpp"


/**
 * Represents an expression of the language, must be inherited.
*/
struct Expression {

    virtual ~Expression() = default;

    /**
     * The position of the expression in the source.
    */
    std::shared_ptr<Parser::Position> position;

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

struct FunctionCall: public Expression {

    std::shared_ptr<Expression> function;
    std::shared_ptr<Expression> arguments;

    FunctionCall() {}

    FunctionCall(std::shared_ptr<Expression> function, std::shared_ptr<Expression> arguments):
        function(function), arguments(arguments) {}

};

struct FunctionDefinition: public Expression {

    std::shared_ptr<Expression> parameters;
    std::shared_ptr<Expression> filter;
    std::shared_ptr<Expression> body;

    FunctionDefinition() {}

    FunctionDefinition(std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> filter, std::shared_ptr<Expression> body):
        parameters(parameters), filter(filter), body(body) {}

};

struct Property: public Expression {

    std::shared_ptr<Expression> object;
    std::string name;

    Property() {}

    Property(std::shared_ptr<Expression> object, std::string name):
        object(object), name(name) {}

};

struct Symbol: public Expression {

    std::string name;

    Symbol() {}

    Symbol(std::string name):
        name(name) {}

};

struct Tuple: public Expression {

    std::vector<std::shared_ptr<Expression>> objects;

    Tuple() {}

    Tuple(std::vector<std::shared_ptr<Expression>> objects):
        objects(objects) {}

};


#endif
