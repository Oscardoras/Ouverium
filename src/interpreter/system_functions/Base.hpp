#ifndef __INTERPRETER_SYSTEMFUNCTIONS_BASE_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_BASE_HPP__

#include "../Interpreter.hpp"


namespace Interpreter {

    namespace Base {

        Reference separator(FunctionContext & context);

        Reference if_statement(FunctionContext & context);

        Reference while_statement(FunctionContext & context);

        Reference for_statement(FunctionContext & context);

        Reference for_step_statement(FunctionContext & context);

        class Exception {

        public:

            Reference reference;
            std::shared_ptr<Parser::Position> position;

        };

        Reference try_statement(FunctionContext & context);

        Reference throw_statement(FunctionContext & context);

        std::string get_canonical_path(FunctionContext & context);

        Reference import(FunctionContext & context);

        Reference copy(FunctionContext & context);
        Reference copy_pointer(FunctionContext & context);

        void assign1(std::vector<Object*> & cache, Reference const& var, Object* const& object);
        void assign2(Context & context, std::vector<Object*>::iterator & it, Reference const& var);
        Reference assign(FunctionContext & context);

        Reference function_definition(FunctionContext & context);

        Reference getter(FunctionContext & context);

        bool equals(Data a, Data b);

        Reference equals(FunctionContext & context);

        Reference not_equals(FunctionContext & context);

        Reference check_pointers(FunctionContext & context);

        Reference not_check_pointers(FunctionContext & context);

        void init(Context & context);

    }

}


#endif
