#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <cstddef>
#include <string>
#include <variant>


#define INT long
#define FLOAT double

/**
 * Gets a data from a symbol name.
 * @param name the symbol name to parse.
 * @return a variant containing the data, null if the data is a variable.
*/
std::variant<std::nullptr_t, bool, INT, FLOAT, std::string> get_symbol(std::string const& name);


#endif
