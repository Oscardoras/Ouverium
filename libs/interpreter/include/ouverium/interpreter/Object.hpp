#ifndef __INTERPRETER_OBJECT_HPP__
#define __INTERPRETER_OBJECT_HPP__

// IWYU pragma: private; include "Interpreter.hpp"

#include <initializer_list>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "Data.hpp"
#include "Function.hpp"

namespace Interpreter {

    struct Object {

        std::map<std::string, Data> properties;
        std::list<Function> functions;
        std::vector<Data> array;

        Object() = default;
        Object(std::initializer_list<Data> const& array);
        Object(std::string const& str);

        [[nodiscard]] std::string to_string() const;

    };

}


#endif
