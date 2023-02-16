#ifndef __PARSER_POSITION_HPP__
#define __PARSER_POSITION_HPP__

#include <memory>
#include <string>


namespace Parser {

    class Context;

    struct Position {
        /**
         * The file path.
        */
        std::string path;

        /**
         * Stores a stack trace from a context.
         * @param context the context to store.
        */
        virtual void store_stack_trace(Context & context) = 0;

        /**
         * Prints an error.
         * @param message a message to notify.
        */
        virtual void notify_error(std::string const& message = "An error occured", bool print_stack_trace = true) = 0;
    };

    class Context {

    protected:

        std::shared_ptr<Parser::Position> position;

    public:

        Context(std::shared_ptr<Parser::Position> position):
            position(position) {}

        auto get_position() const {
            return position;
        }

        virtual Context & get_parent() = 0;

    };

}


#endif
