#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <cstddef>
#include <string>
#include <variant>

#include "types.h"


/**
 * Gets a data from a symbol name.
 * @param name the symbol name to parse.
 * @return a variant containing the data, null if the data is a variable.
*/
std::variant<std::nullptr_t, bool, OV_INT, OV_FLOAT, std::string> get_symbol(std::string const& name);


#endif
