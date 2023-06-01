#ifndef __COMPILER_ANALYZER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_ANALYZER_HPP__

#include "Expressions.hpp"

#include "../parser/Parser.hpp"


namespace Analyzer {

    class Analyzer {

    public:

        virtual std::pair<std::shared_ptr<Expression>, MetaData> analyze(std::shared_ptr<Parser::Expression> expression) const = 0;

    };

}


#endif
