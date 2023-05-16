#ifndef __COMPILER_ANALYZER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_ANALYZER_HPP__

#include "AnalyzedExpressions.hpp"

#include "../parser/Parser.hpp"


namespace Analyzer {

    class Analyzer {

    public:

        virtual std::shared_ptr<AnalyzedExpression> analyze(std::shared_ptr<Parser::Expression> expression) const = 0;

    };

}


#endif
