#ifndef __INTERPRETER_GC_HPP__
#define __INTERPRETER_GC_HPP__

#include "Object.hpp"


namespace Interpreter::GC {

    void add_context(Context& context);
    void remove_context(Context& context);

    ObjectPtr new_object(Object const& object = {});
    SymbolReference new_reference(Data const& data = {});

    void collect();

}


#endif
