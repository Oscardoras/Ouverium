#include <charconv>
#include <iostream>

#include "Types.hpp"
#include "ouverium/types.h"


std::variant<std::nullptr_t, bool, OV_INT, OV_FLOAT, std::string> get_symbol(std::string const& name) {
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

    OV_INT i{};
    if (std::from_chars(name.data(), name.data() + name.size(), i).ec == std::errc())
        return static_cast<OV_INT>(i);

    OV_FLOAT f{};
    if (std::from_chars(name.data(), name.data() + name.size(), f).ec == std::errc())
        return static_cast<OV_FLOAT>(f);

    // std::cerr << name << std::endl;

    return nullptr;
}
