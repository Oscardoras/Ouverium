#include "Interpreter.hpp"


namespace Interpreter {

    Object::Object(std::string const& str) {
        array.reserve(str.size());
        for (auto c : str)
            array.push_back(Data(c));
    }

    std::string Object::to_string() const {
        std::string str;

        for (auto d : array)
            str.push_back(d.get<char>());

        return str;
    }

}
