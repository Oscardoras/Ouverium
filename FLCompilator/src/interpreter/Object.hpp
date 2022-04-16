#ifndef INTERPRETER_OBJECT_HPP_
#define INTERPRETER_OBJECT_HPP_

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct Function;


struct Object {
    
    std::map<std::string, Object*> fields;

    std::list<Function*> functions;

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

    Object(bool b);
    Object(long i);
    Object(double f);
    Object(char c);
    Object(size_t tuple_size);

    ~Object();

};


#endif