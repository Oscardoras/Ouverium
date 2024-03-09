#include "Interpreter.hpp"


namespace Interpreter {

    Data compute(Context& context, std::shared_ptr<Parser::Expression> caller, Reference const& reference) {
        if (auto symbol = std::get_if<SymbolReference>(&reference)) {
            if (*symbol == std::get<SymbolReference>(context.get_global()["getter"]))
                return symbol->get();
        }

        if (auto d = std::get_if<Data>(&reference); d && *d != Data{})
            return *d;
        else
            return call_function(context, caller, context.get_global()["getter"], reference).to_data(context, caller);
    }

    Data IndirectReference::to_data(Context& context, std::shared_ptr<Parser::Expression> caller) const {
        return compute(context, caller, *this);
    }

    Reference::Reference(IndirectReference const& indirect_reference) {
        if (auto symbol_reference = std::get_if<SymbolReference>(&indirect_reference))
            *this = *symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(&indirect_reference))
            *this = *property_reference;
        else if (auto array_reference = std::get_if<ArrayReference>(&indirect_reference))
            *this = *array_reference;
    }

    Data Reference::to_data(Context& context, std::shared_ptr<Parser::Expression> caller) const {
        return compute(context, caller, *this);
    }

    IndirectReference Reference::to_indirect_reference(Context& context, std::shared_ptr<Parser::Expression> caller) const {
        if (auto data = std::get_if<Data>(this))
            return context.new_reference(*data);
        else if (auto symbol_reference = std::get_if<SymbolReference>(this))
            return *symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(this))
            return *property_reference;
        else if (auto array_reference = std::get_if<ArrayReference>(this))
            return *array_reference;
        else if (auto tuple_reference = std::get_if<TupleReference>(this)) {
            auto object = context.new_object();
            object->array.reserve(tuple_reference->size());
            for (auto d : *tuple_reference)
                object->array.push_back(d.to_data(context, caller));
            return context.new_reference(Data(object));
        } else return context.new_reference();
    }
    
}
