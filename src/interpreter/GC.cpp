#include <cassert>
#include <memory>

#include "Interpreter.hpp"


namespace Interpreter::GC {

    ObjectPtr new_object(Object const& object) {
        return std::make_shared<Object>(object);
    }

    SymbolReference new_reference(Data const& data) {
        return { std::make_shared<Data>(data) };
    }

    void collect() {

    }
}
