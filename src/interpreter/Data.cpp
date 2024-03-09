#include "Interpreter.hpp"


namespace Interpreter {

    bool operator==(Data const& a, Data const& b) {
        if (!static_cast<std::any const&>(a).has_value() && !static_cast<std::any const&>(b).has_value())
            return true;
        else if (auto a_object = get_if<Object*>(&a)) {
            if (auto b_object = get_if<Object*>(&b))
                return *a_object == *b_object;
            else return false;
        } else if (auto a_char = get_if<char>(&a)) {
            if (auto b_char = get_if<char>(&b)) return *a_char == *b_char;
            else return false;
        } else if (auto a_float = get_if<OV_FLOAT>(&a)) {
            if (auto b_float = get_if<OV_FLOAT>(&b)) return *a_float == *b_float;
            else return false;
        } else if (auto a_int = get_if<OV_INT>(&a)) {
            if (auto b_int = get_if<OV_INT>(&b)) return *a_int == *b_int;
            else return false;
        } else if (auto a_bool = get_if<bool>(&a)) {
            if (auto b_bool = get_if<bool>(&b)) return *a_bool == *b_bool;
            else return false;
        } else return false;
    }

}
