#ifndef __PARSER_STANDARD_HPP__
#define __PARSER_STANDARD_HPP__

#include "Expressions.hpp"


namespace Parser {

    class Standard {

    public:

        Standard(std::string const& code, std::string const& path);

        struct Word : public std::string {
            std::string position;

            Word(std::string const& word, Parser::Position const& position);
        };

        std::vector<Word> get_words() const;

        struct ParsingError {
            std::string message;
            std::string position;

            ParsingError(std::string const& message, Parser::Position const& position) :
                message(message), position(position) {}
        };

        class Exception : std::exception {

            std::string message;

        public:

            Exception(std::vector<ParsingError> const& errors = {});

            const char* what() const noexcept override;

        };

        class IncompleteCode : public Exception {};

        std::shared_ptr<Expression> get_tree() const;

    protected:

        std::string code;
        std::string path;

    };

}


#endif
