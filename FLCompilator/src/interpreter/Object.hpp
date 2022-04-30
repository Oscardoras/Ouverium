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
        CPointer = -5,
        Char,
        Float,
        Int,
        Bool,
        None = 0
        //Array > 0
    };
    
    long type;

    union Data {
        void* ptr;
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
    Object(void* ptr);
    Object(long i);
    Object(double f);
    Object(char c);
    Object(size_t tuple_size);

    ~Object();

    std::string toString() const;

};


#endif