#ifndef PARSER_POSITION_HPP_
#define PARSER_POSITION_HPP_


struct Position {
    std::string path;
    virtual void notify() = 0;
};


#endif