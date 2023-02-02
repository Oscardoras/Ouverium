#include "Context.hpp"
#include "Reference.hpp"


namespace Interpreter {

    Data Reference::to_data(Context & context) const {
        if (auto data = std::get_if<Data>(this)) return *data;
        else if (auto symbol_reference = std::get_if<SymbolReference>(this)) return *symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(this)) return *property_reference;
        else if (auto array_reference = std::get_if<ArrayReference>(this)) return *array_reference;
        else if (auto tuple_reference = std::get_if<TupleReference>(this)) {
            auto object = context.new_object();
            for (auto d : *tuple_reference)
                object->array.push_back(d.to_data(context));
            return object;
        } else return Data{};
    }

    Data & Reference::to_symbol_reference(Context & context) const {
        if (auto symbol_reference = std::get_if<SymbolReference>(this)) return symbol_reference->get();
        else {
            SymbolReference reference = context.new_reference();
            reference.get() = to_data(context);
            return reference;
        }
    }

}
