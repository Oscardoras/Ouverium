#include "Context.hpp"
#include "Reference.hpp"


namespace Interpreter {

    Data Reference::to_data(Context & context) const {
        if (auto data = std::get_if<Data>(this)) return *data;
        else if (auto symbol_reference = std::get_if<SymbolReference>(this)) return **symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(this)) return *property_reference->reference;
        else if (auto array_reference = std::get_if<ArrayReference>(this)) return array_reference->array->array[array_reference->i];
        else if (auto tuple_reference = std::get_if<TupleReference>(this)) {
            auto object = context.new_object();
            for (auto d : *tuple_reference)
                object->array.push_back(d.to_data(context));
            return object;
        }
    }

    SymbolReference Reference::to_symbol_reference(Context & context) {
        if (auto symbol_reference = std::get_if<SymbolReference>(this)) return symbol_reference;
        else {
            SymbolReference symbol_reference = context.symbols;
        }
        else if (auto data = std::get_if<Data>(this)) return *data;
        else if (auto symbol_reference = std::get_if<SymbolReference>(this)) return **symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(this)) return *property_reference->reference;
        else if (auto array_reference = std::get_if<ArrayReference>(this)) return array_reference->array->array[array_reference->i];
        else if (auto tuple_reference = std::get_if<TupleReference>(this)) {
            auto object = context.new_object();
            for (auto d : *tuple_reference)
                object->array.push_back(d.to_data(context));
            return object;
        }
    }

}
