#ifndef __INTERPRETER_OBJECT_HPP__
#define __INTERPRETER_OBJECT_HPP__

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct Context;
struct Function;


struct Object {

    std::map<std::string, Object*> properties;

    std::list<std::unique_ptr<Function>> functions;

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
        } *a;
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

    Object* &get_property(std::string name, Context & context);
    std::string to_string() const;

};


#endif
