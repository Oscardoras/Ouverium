#ifndef __PARSER_STANDARD_HPP__
#define __PARSER_STANDARD_HPP__

#include <list>

#include "../Expressions.hpp"


namespace Parser {

    namespace Standard {

        struct TextPosition: public Position {
            unsigned int line;
            unsigned int column;
            std::string stack_trace;

            TextPosition(TextPosition const&) = default;
            TextPosition(std::string const& path, unsigned int line, unsigned int column);

            virtual void store_stack_trace(Context & context) override;
            virtual void notify_error(std::string const& message, bool print_stack_trace) override;
        };

        struct Word {
            std::string word;
            TextPosition position;

            Word(std::string const& word, TextPosition const& position);
        };

        std::vector<Word> get_words(std::string const& path, std::string const& code);

        struct ParserError {
            std::string message;
            TextPosition position;

            ParserError(std::string const& message, TextPosition const& position);
        };

        class IncompleteCode: public std::exception {};

        std::shared_ptr<Expression> get_tree(std::string const& code, std::string const& path, std::vector<std::string> symbols);

    }

}


#endif
