#ifndef COMPILER_JAVASCRIPT_HPP_
#define COMPILER_JAVASCRIPT_HPP_

#include "../../parser/expression/Expression.hpp"


namespace JavascriptTranslator {

    std::string getJavaScript(std::shared_ptr<Parser::Expression> expression);

}

#endif