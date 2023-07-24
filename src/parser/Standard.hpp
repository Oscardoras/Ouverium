#ifndef __PARSER_STANDARD_HPP__
#define __PARSER_STANDARD_HPP__

#include <iostream>
#include <list>

#include "Parser.hpp"


namespace Parser {

    class Standard {

    public:

        Standard(std::string const& code, std::string const& path);

        struct TextPosition: public Position {
            std::string path;
            unsigned int line;
            unsigned int column;
            std::string stack_trace;

            TextPosition(TextPosition const&) = default;
            TextPosition(std::string const& path, unsigned int line, unsigned int column);

            virtual void notify_error(std::string const& message) const override;
            virtual void notify_position() const override;
        };

        struct Word: public std::string {
            TextPosition position;

            Word(std::string const& word, TextPosition const& position);
        };

        std::vector<Word> get_words() const;

        struct ParsingError {
            std::string message;
            TextPosition position;

            ParsingError(std::string const& message, TextPosition const& position):
                message(message), position(position) {}
        };

        class Exception : std::exception {

            std::string message;

        public:

            Exception(std::vector<ParsingError> const& errors = {});

            virtual const char* what() const noexcept override;

        };

        class IncompleteCode: public Exception {};

        std::shared_ptr<Expression> get_tree() const;

    protected:

        std::string code;
        std::string path;

    };

}


#endif
