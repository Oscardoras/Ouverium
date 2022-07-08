#ifndef __PARSER_POSITION_HPP__
#define __PARSER_POSITION_HPP__


struct Position {
    std::string path;
    virtual void notify() = 0;
};


#endif
