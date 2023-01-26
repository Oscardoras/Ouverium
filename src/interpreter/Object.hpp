#ifndef __INTERPRETER_OBJECT_HPP__
#define __INTERPRETER_OBJECT_HPP__

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Data.hpp"


namespace Interpreter {

    struct Context;
    struct Function;

    struct Object {

        std::map<std::string, Object*> properties;
        std::list<std::unique_ptr<Function>> functions;
        union ArrayElement {
            size_t c;
            Data o;
        } *array = nullptr;

        bool referenced = false;

        Object() = default;
        Object(size_t tuple_size);

        ~Object();

        Object*& get_property(std::string name, Context & context);
        std::string to_string() const;

    };

}


#endif
