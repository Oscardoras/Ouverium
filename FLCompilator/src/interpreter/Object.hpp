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
        Float = -3,
        Integer,
        Boolean,
        None = 0
        //Array > 0
    };
    
    long type;

    union Data {
        double f;
        long i;
        bool b;
        union ArrayElement {
            size_t c;
            Object* o;
        } * a;
    } data;

    int references;

    Object();

    Object(Object const& object);

    Object(bool const& b);
    Object(long const& i);
    Object(double const& f);
    Object(size_t const& tuple_size);

    ~Object();

    void addReference();

    void removeReference();

};


#endif