#ifndef __PARSER_STANDARD_HPP__
#define __PARSER_STANDARD_HPP__

#include <utility>

#include "Expressions.hpp"


namespace Parser {

    class Standard {

    public:

        Standard(std::string code, std::string path);

        struct Word : public std::string {
            std::string position;

            Word(std::string word, Parser::Position position);
        };

        std::vector<Word> get_words() const;

        struct ParsingError {
            std::string message;
            std::string position;

            ParsingError(std::string message, Parser::Position position) :
                message(std::move(message)), position(std::move(position)) {}
        };

        class Exception : std::exception {

            std::string message;

        public:

            explicit Exception(std::vector<ParsingError> const& errors = {});

            [[nodiscard]] const char* what() const noexcept override;

        };

        class IncompleteCode : public Exception {};

        std::shared_ptr<Expression> get_tree() const;

    protected:

        std::string code;
        std::string path;

    };

}


#endif
