#ifndef INTERPRETER_OBJECT_HPP_
#define INTERPRETER_OBJECT_HPP_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Function.hpp"


struct Object {
    
    std::map<std::string, Object*> fields;

    Function* function;

    enum ObjectType {
        Char = -4,
        Float,
        Integer,
        Boolean,
        None = 0
        //Array > 0
    };
    
    long type;

    union Data {
        char c;
        double f;
        long i;
        bool b;
        union ArrayElement {
            size_t c;
            Object* o;
        } * a;
    } data;

    bool referenced;

    Object();

    Object(Object const& object);

    Object(bool const& b);
    Object(long const& i);
    Object(double const& f);
    Object(char const& c);
    Object(size_t const& tuple_size);

};


#endif