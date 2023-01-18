#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <variant>


std::variant<nullptr_t, bool, long, double, std::string> get_symbol(std::string name);


#endif
