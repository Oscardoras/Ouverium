#ifndef __PARSER_POSITION_HPP__
#define __PARSER_POSITION_HPP__

#include <string>

struct Context;


struct Position {
    std::string path;
    
    virtual void get_stack_trace(Context & context) = 0;
    virtual void error() = 0;
};


#endif
