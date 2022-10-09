#ifndef __PARSER_POSITION_HPP__
#define __PARSER_POSITION_HPP__

#include <string>

struct Context;


struct Position {
    std::string path; // The file path.

    virtual void store_stack_trace(Context & context) = 0; // Stores a stack trace from a context.
    virtual void notify_error() = 0; // Prints an error.
};


#endif
