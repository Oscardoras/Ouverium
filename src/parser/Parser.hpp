#ifndef __PARSER_PARSER_HPP__
#define __PARSER_PARSER_HPP__

#include "Expressions.hpp"


namespace Parser {

    class Context;
    class Expression;

    class Position {

    public:

        /**
         * Stores a stack trace from a context.
         * @param context the context to store.
        */
        virtual void store_stack_trace(Context & context) = 0;

        /**
         * Prints an error.
         * @param message a message to notify.
         * @param print_stack_trace if the stack trace musts be printed.
        */
        virtual void notify_error(std::string const& message = "An error occured", bool print_stack_trace = true) = 0;

    };

    class Context {

    protected:

        /**
         * The position from where the context was called.
        */
        std::shared_ptr<Position> position;

    public:

        Context(std::shared_ptr<Position> position):
            position(position) {}

        /**
         * Gets the position where the context was called.
         * @return the position.
        */
        auto get_position() const {
            return position;
        }

        /**
         * Gets the parent context.
         * @return the parent context.
        */
        virtual Context & get_parent() = 0;

    };

    class Parser {

    public:

        virtual std::shared_ptr<Expression> get_tree(std::set<std::string> symbols) = 0;

    };

}


#endif
