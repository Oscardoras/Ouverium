#ifndef __PARSER_STANDARD_HPP__
#define __PARSER_STANDARD_HPP__

#include <list>

#include "Parser.hpp"


namespace Parser {

    class Standard: public Parser {

    public:

        Standard(std::string const& code, std::string const& path);

        struct TextPosition: public Position {
            std::string path;
            unsigned int line;
            unsigned int column;
            std::string stack_trace;

            TextPosition(TextPosition const&) = default;
            TextPosition(std::string const& path, unsigned int line, unsigned int column);

            virtual void store_stack_trace(Context & context) override;
            virtual void notify_error(std::string const& message, bool print_stack_trace) override;
        };

        struct Word: public std::string {
            TextPosition position;

            Word(std::string const& word, TextPosition const& position);
        };

        std::vector<Word> get_words();

        struct ParserError {
            std::string message;
            TextPosition position;

            ParserError(std::string const& message, TextPosition const& position);
        };

        class IncompleteCode: public std::exception {};

        virtual std::shared_ptr<Expression> get_tree(std::set<std::string> symbols) override;

    protected:

        std::string code;
        std::string path;

        std::shared_ptr<Expression> get_tree(std::vector<Standard::ParserError> & errors, std::set<std::string> symbols);

    };

}


#endif
