#ifndef __INTERPRETER_SYSTEMFUNCTIONS_STRING_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_STRING_HPP__

#include "ArrayList.hpp"


namespace Interpreter {

    namespace String {

        std::shared_ptr<Parser::Expression> string_p();
        Reference char_constructor(FunctionContext & context);

        std::shared_ptr<Parser::Expression> char_is_p();

        Reference char_is_digit(FunctionContext & context);
        Reference char_is_alpha(FunctionContext & context);
        Reference char_is_alphanum(FunctionContext & context);

        Reference string(FunctionContext & context);

        std::shared_ptr<Parser::Expression> index_of_p();
        Reference index_of(FunctionContext & context);

        std::shared_ptr<Parser::Expression> substring_p();
        Reference substring(FunctionContext & context);

        std::shared_ptr<Parser::Expression> includes_p();
        Reference includes(FunctionContext & context);

        std::shared_ptr<Parser::Expression> concat_p();
        Reference concat(FunctionContext & context);

        std::shared_ptr<Parser::Expression> assign_concat_p();
        Reference assign_concat(FunctionContext & context);

        void init(GlobalContext & context);

    }

}


#endif
