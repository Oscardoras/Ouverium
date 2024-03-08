#ifndef __PARSER_EXPRESSIONS_HPP__
#define __PARSER_EXPRESSIONS_HPP__

#include <memory>
#include <set>
#include <string>
#include <vector>


namespace Parser {

    using Position = std::string;

    /**
     * Represents an expression of the language, must be inherited.
    */
    struct Expression : std::enable_shared_from_this<Expression> {

        /**
         * The parent expression.
        */
        std::weak_ptr<Expression> parent;

        /**
         * Gets the root expression.
         * @return the root expression.
        */
        std::shared_ptr<Expression> get_root();

        /**
         * The position of the expression in the source.
        */
        Position position;

        /**
         * The list of the symbols used in the expression.
        */
        std::set<std::string> symbols;

        /**
         * Gets all the symbols present in this expression.
         * @return the symbols present in this expression.
        */
        virtual std::set<std::string> get_symbols() const = 0;

        /**
         * Determine the symbols of this expressions tree.
         * @param available_symbols the symbols available in the parent expression.
         * @return the symbols used by this expression.
        */
        virtual std::set<std::string> compute_symbols(std::set<std::string>& available_symbols) = 0;

        /**
         * Gets a string of the expression to print it as a tree.
         * @return the expression as a string.
        */
        virtual std::string to_string(unsigned n = 0) const = 0;

        virtual ~Expression() = default;

    };

    struct FunctionCall : public Expression {

        std::shared_ptr<Expression> function;
        std::shared_ptr<Expression> arguments;

        FunctionCall(std::shared_ptr<Expression> function = nullptr, std::shared_ptr<Expression> arguments = nullptr) :
            function(function), arguments(arguments) {}

        std::set<std::string> get_symbols() const override;
        std::set<std::string> compute_symbols(std::set<std::string>& available_symbols) override;
        std::string to_string(unsigned int n = 0) const override;

    };

    struct FunctionDefinition : public Expression {

        std::set<std::string> captures;

        std::shared_ptr<Expression> parameters;
        std::shared_ptr<Expression> filter;
        std::shared_ptr<Expression> body;

        FunctionDefinition(std::shared_ptr<Expression> parameters = nullptr, std::shared_ptr<Expression> filter = nullptr, std::shared_ptr<Expression> body = nullptr) :
            parameters(parameters), filter(filter), body(body) {}

        std::set<std::string> get_symbols() const override;
        std::set<std::string> compute_symbols(std::set<std::string>& available_symbols) override;
        std::string to_string(unsigned int n = 0) const override;

    };

    struct Property : public Expression {

        std::shared_ptr<Expression> object;
        std::string name;

        Property(std::shared_ptr<Expression> object = nullptr, std::string const& name = "") :
            object(object), name(name) {}

        std::set<std::string> get_symbols() const override;
        std::set<std::string> compute_symbols(std::set<std::string>& available_symbols) override;
        std::string to_string(unsigned int n = 0) const override;

    };

    struct Symbol : public Expression {

        std::string name;

        Symbol(std::string const& name = "") :
            name(name) {}

        std::set<std::string> get_symbols() const override;
        std::set<std::string> compute_symbols(std::set<std::string>& available_symbols) override;
        std::string to_string(unsigned int n = 0) const override;

    };

    struct Tuple : public Expression {

        std::vector<std::shared_ptr<Expression>> objects;

        Tuple(std::initializer_list<std::shared_ptr<Expression>> const& l = {}) :
            objects(l) {}

        std::set<std::string> get_symbols() const override;
        std::set<std::string> compute_symbols(std::set<std::string>& available_symbols) override;
        std::string to_string(unsigned int n = 0) const override;

    };

}


#endif
