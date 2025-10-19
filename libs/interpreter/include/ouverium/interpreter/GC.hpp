#pragma once

// IWYU pragma: private; include "Interpreter.hpp"

#include "Data.hpp"
#include "Object.hpp"
#include "Reference.hpp"


namespace Interpreter::GC {

    [[nodiscard]] ObjectPtr new_object(Object object = {});
    [[nodiscard]] SymbolReference new_reference(Data data = {});

    void collect();

}
