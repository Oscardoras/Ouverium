#ifndef __COMPILER_FUNCTIONS_HPP__
#define __COMPILER_FUNCTIONS_HPP__

#include "Analyzer.hpp"


namespace Analyzer {

    namespace Functions {

        M<Reference> separator(Context & context);

        void assignation(M<Reference> const& var, M<Data> const& data, bool potential);

    }

}


#endif
