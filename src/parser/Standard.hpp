#ifndef __PARSER_STANDARD_HPP__
#define __PARSER_STANDARD_HPP__

#include <list>

#include "expression/Expression.hpp"

#include "Position.hpp"


namespace StandardParser {

    struct TextPosition: public Position {
        int line;
        int column;
        std::string stack_trace;

        TextPosition(TextPosition const&) = default;
        TextPosition(std::string const& path, int line, int column);

        virtual void get_stack_trace(Context & context) override;
        virtual void error() override;
    };

    struct Word {
        std::string word;
        TextPosition position;

        Word(std::string const& word, TextPosition position);
    };

    std::vector<Word> get_words(std::string const& path, std::string const& code);

    struct ParserError {
        std::string message;
        TextPosition position;

        ParserError(std::string const& message, TextPosition position);
    };

    struct IncompleteCode {};

    std::shared_ptr<Expression> get_tree(std::vector<ParserError> & errors, std::string const& path, std::string const& code, std::vector<std::string> symbols);

    std::shared_ptr<Expression> get_tree(std::string const& code, std::string const& path, std::vector<std::string> symbols);

}


#endif
