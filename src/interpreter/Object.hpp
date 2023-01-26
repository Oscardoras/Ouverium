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

        std::vector<Data> array;

        bool referenced = false;

        Object* & get_property(std::string name, Context & context);
        std::string to_string() const;

    };

}


#endif
