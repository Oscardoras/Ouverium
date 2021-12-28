#ifndef PARSER_STANDARD_HPP_
#define PARSER_STANDARD_HPP_

#include "expression/Expression.hpp"


namespace StandardParser {

    std::shared_ptr<Expression> getTree(std::string code, std::vector<std::string> symbols);

}

#endif