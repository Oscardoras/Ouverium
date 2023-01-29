#ifndef __PARSER_POSITION_HPP__
#define __PARSER_POSITION_HPP__

#include <string>


namespace Interpreter {
    struct Context;
}

namespace Parser {

    struct Position {
        /**
         * The file path.
        */
        std::string path;

        /**
         * Stores a stack trace from a context.
         * @param context the context to store.
        */
        virtual void store_stack_trace(Interpreter::Context & context) = 0;

        /**
         * Prints an error.
         * @param message a message to notify.
        */
        virtual void notify_error(std::string const& message = "An error occured", bool print_stack_trace = true) = 0;
    };

}


#endif
