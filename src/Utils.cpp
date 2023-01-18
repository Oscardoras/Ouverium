#include <stdexcept>

#include "Utils.hpp"


std::variant<nullptr_t, bool, long, double, std::string> get_symbol(std::string name) {
    if (name[0] == '\"') {
        std::string str;

        bool escape = false;
        bool first = true;
        for (char c : name) if (!first) {
            if (!escape) {
                if (c == '\"') break;
                else if (c == '\\') escape = true;
                else str += c;
            } else {
                escape = false;
                if (c == 'b') str += '\b';
                if (c == 'e') str += '\e';
                if (c == 'f') str += '\f';
                if (c == 'n') str += '\n';
                if (c == 'r') str += '\r';
                if (c == 't') str += '\t';
                if (c == 'v') str += '\v';
                if (c == '\\') str += '\\';
                if (c == '\'') str += '\'';
                if (c == '\"') str += '\"';
                if (c == '?') str += '\?';
            }
        } else first = false;

        return str;
    }
    if (name == "true") return true;
    if (name == "false") return false;
    try {
        return std::stol(name);
    } catch (std::invalid_argument const& ex1) {
        try {
            return std::stod(name);
        } catch (std::invalid_argument const& ex2) {
            return nullptr;
        }
    }
}
