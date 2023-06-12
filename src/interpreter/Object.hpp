#ifndef __INTERPRETER_OBJECT_HPP__
#define __INTERPRETER_OBJECT_HPP__

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Data.hpp"


namespace Interpreter {

    struct Context;

    struct Object {

        std::list<Object*> prototypes;

        std::map<std::string, Data> properties;
        std::list<Function> functions;
        std::vector<Data> array;
        struct CPointer: public std::variant<std::shared_ptr<std::ios>, std::reference_wrapper<std::ios>> {
            using std::variant<std::shared_ptr<std::ios>, std::reference_wrapper<std::ios>>::variant;

            inline operator std::ios*() const {
                if (auto ptr = std::get_if<std::shared_ptr<std::ios>>(this))
                    return ptr->get();
                else if (auto ref = std::get_if<std::reference_wrapper<std::ios>>(this))
                    return &ref->get();
                else return nullptr;
            }
        } c_pointer = nullptr;

        bool referenced = false;

        Data & get_property(std::string name, Context & context);
        std::string to_string() const;

    };

}


#endif
