#ifndef __INTERPRETER_GC_HPP__
#define __INTERPRETER_GC_HPP__

// IWYU pragma: private; include "Interpreter.hpp"

#include "Data.hpp"
#include "Object.hpp"
#include "Reference.hpp"


namespace Interpreter::GC {

    [[nodiscard]] ObjectPtr new_object(Object const& object = {});
    [[nodiscard]] SymbolReference new_reference(Data const& data = {});

    void collect();

}


#endif
