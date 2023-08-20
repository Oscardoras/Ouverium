#ifndef __PARSER_PARSER_HPP__
#define __PARSER_PARSER_HPP__

#include "Expressions.hpp"


namespace Parser {

    class Position {

    public:

        /**
         * Prints an error.
         * @param message a message to notify.
        */
        virtual void notify_error(std::string const& message) const = 0;

        /**
         * Prints the position.
        */
        virtual void notify_position() const = 0;

    };

}


#endif
