#ifndef __COMPILER_C_SYSTEMFUNCTIONS_BASE_HPP__
#define __COMPILER_C_SYSTEMFUNCTIONS_BASE_HPP__

#include "../Interpreter.hpp"


namespace Base {

    std::shared_ptr<Expression> separator();
    Reference separator(FunctionContext & context);

    std::shared_ptr<Expression> if_statement();
    Reference if_statement(FunctionContext & context);

    std::shared_ptr<Expression> while_statement();
    Reference while_statement(FunctionContext & context);

    std::shared_ptr<Expression> for_statement();
    Reference for_statement(FunctionContext & context);

    std::shared_ptr<Expression> for_step_statement();
    Reference for_step_statement(FunctionContext & context);

    struct Exception {
        Reference reference;
        std::shared_ptr<Position> position;
    };

    std::shared_ptr<Expression> try_statement();
    Reference try_statement(FunctionContext & context);

    std::shared_ptr<Expression> throw_statement() ;
    Reference throw_statement(FunctionContext & context);

    std::string get_canonical_path(FunctionContext & context);

    std::shared_ptr<Expression> path();

    Reference include(FunctionContext & context);

    Reference use(FunctionContext & context);

    std::shared_ptr<Expression> copy();
    Reference copy(FunctionContext & context);
    Reference copy_pointer(FunctionContext & context);

    void assign1(std::vector<Object*> & cache, Reference const& var, Object* const& object);

    void assign2(Context & context, std::vector<Object*>::iterator & it, Reference const& var);

    std::shared_ptr<Expression> assign();
    Reference assign(FunctionContext & context);

    std::shared_ptr<Expression> function_definition();
    Reference function_definition(FunctionContext & context);

    bool equals(Object* a, Object* b);

    std::shared_ptr<Expression> equality();

    Reference equals(FunctionContext & context);

    Reference not_equals(FunctionContext & context);

    Reference check_pointers(FunctionContext & context);

    Reference not_check_pointers(FunctionContext & context);

    void init(Context & context);

}


#endif
