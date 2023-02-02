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

        std::map<std::string, Data> properties;
        std::list<Function> functions;
        std::vector<Data> array;
        class CPointer: public std::variant<std::unique_ptr<std::ios>, std::reference_wrapper<std::ios>> {
            public:

            using std::variant<std::unique_ptr<std::ios>, std::reference_wrapper<std::ios>>::variant;

            operator std::ios*() const {
                if (auto ptr = std::get_if<std::unique_ptr<std::ios>>(this))
                    return ptr->get();
                else if (auto ref = std::get_if<std::reference_wrapper<std::ios>>(this))
                    return &ref->get();
                else return nullptr;
            }
        };
        CPointer c_pointer = nullptr;

        bool referenced = false;

        Data & get_property(std::string name, Context & context);
        std::string to_string() const;

        ~Object();

    };

}


#endif
