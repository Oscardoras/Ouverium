#ifndef __PARSER_EXPRESSIONS_HPP__
#define __PARSER_EXPRESSIONS_HPP__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Position.hpp"


namespace Parser {

    /**
     * Represents an expression of the language, must be inherited.
    */
    struct Expression {

        virtual ~Expression() = default;

        /**
         * The position of the expression in the source.
        */
        std::shared_ptr<Position> position;

        /**
         * The list of the symbols available in the expression.
        */
        std::set<std::string> symbols;

        /**
         * Gets a string of the expression to print it as a tree.
         * @return the expression as a string.
        */
        std::string to_string() const;

    protected:

        Expression() = default;

    };

    struct FunctionCall: public Expression {

        std::shared_ptr<Expression> function;
        std::shared_ptr<Expression> arguments;

        FunctionCall(std::shared_ptr<Expression> function = nullptr, std::shared_ptr<Expression> arguments = nullptr):
            function(function), arguments(arguments) {}

    };

    struct FunctionDefinition: public Expression {

        std::shared_ptr<Expression> parameters;
        std::shared_ptr<Expression> filter;
        std::shared_ptr<Expression> body;

        FunctionDefinition(std::shared_ptr<Expression> parameters = nullptr, std::shared_ptr<Expression> filter = nullptr, std::shared_ptr<Expression> body = nullptr):
            parameters(parameters), filter(filter), body(body) {}

    };

    struct Property: public Expression {

        std::shared_ptr<Expression> object;
        std::string name;

        Property(std::shared_ptr<Expression> object = nullptr, std::string const& name = ""):
            object(object), name(name) {}

    };

    struct Symbol: public Expression {

        std::string name;

        Symbol(std::string const& name = ""):
            name(name) {}

    };

    struct Tuple: public Expression {

        std::vector<std::shared_ptr<Expression>> objects;

        Tuple(std::initializer_list<std::shared_ptr<Expression>> const& l = {}):
            objects(l) {}

    };

}


#endif
