#ifndef __COMPILER_FUNCTIONS_HPP__
#define __COMPILER_FUNCTIONS_HPP__

#include "Analyzer.hpp"


namespace Analyzer {

    namespace Functions {

        M<Reference> separator(Context & context);

        M<Reference> if_statement(Context & context, bool potential);

        M<Reference> while_statement(Context & context, bool potential);

        M<Reference> for_statement(Context & context, bool potential);

        M<Reference> for_step_statement(Context & context, bool potential);

        class Exception {

        public:

            M<Reference> reference;
            std::shared_ptr<Parser::Position> position;

        };

        M<Reference> try_statement(Context & context, bool potential);

        M<Reference> throw_statement(Context & context);

        M<Reference> include(Context & context, bool potential);

        M<Reference> use(Context & context, bool potential);

        M<Reference> copy(Context & context, bool potential);

        M<Reference> copy_pointer(Context & context, bool potential);

        M<Reference> assign(Context & context, bool potential);

        M<Reference> function_definition(Context & context, bool potential);

        M<Reference> equals(Context & context, bool potential);

        M<Reference> not_equals(Context & context, bool potential);

        M<Reference> check_pointers(Context & context, bool potential);

        M<Reference> not_check_pointers(Context & context, bool potential);

    }

}


#endif
