#include <initializer_list>
#include <string>
#include <vector>

#include "Interpreter.hpp"


namespace Interpreter {

    Object::Object(std::initializer_list<Data> const& array) :
        array{ array } {}

    Object::Object(std::string const& str) {
        array.reserve(str.size());

        for (auto c : str)
            array.emplace_back(c);
    }

    std::string Object::to_string() const {
        std::string str;
        str.reserve(array.size() + 1);

        for (auto const& d : array)
            str.push_back(d.get<char>());

        return str;
    }

}
