#ifndef __PARSER_STANDARD_HPP__
#define __PARSER_STANDARD_HPP__

#include "expression/Expression.hpp"

#include "Position.hpp"


namespace StandardParser {

    struct TextPosition: public Position {
        int line;
        int column;

        TextPosition(TextPosition const&) = default;
        TextPosition(std::string const& path, int line, int column);

        virtual void notify();
    };

    struct Word {
        std::string word;
        TextPosition position;

        Word(std::string const& word, TextPosition position);
    };

    struct ParserError {
        std::string message;
        TextPosition position;

        ParserError(std::string const& message, TextPosition position);
    };

    struct IncompleteError {};

    std::vector<Word> get_words(std::string const& path, std::string const& code);

    std::shared_ptr<Expression> get_tree(std::vector<ParserError> & errors, std::string const& path, std::string const& code, std::vector<std::string> symbols);

}


#endif
