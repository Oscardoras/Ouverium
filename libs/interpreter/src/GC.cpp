#include <cassert>
#include <memory>
#include <utility>

#include <ouverium/interpreter/Interpreter.hpp>


namespace Interpreter::GC {

    ObjectPtr new_object(Object object) {
        return std::make_shared<Object>(std::move(object));
    }

    SymbolReference new_reference(Data data) {
        return { std::make_shared<Data>(std::move(data)) };
    }

    void collect() {}
}
