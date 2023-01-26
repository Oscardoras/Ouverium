#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <variant>


/**
 * Gets a data from a symbol name.
 * @param name the symbol name to parse.
 * @return a variant containing the data, null if the data is a variable.
*/
std::variant<nullptr_t, bool, long, double, std::string> get_symbol(std::string name);


#endif
