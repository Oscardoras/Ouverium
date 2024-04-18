#include "Interpreter.hpp"


template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

namespace Interpreter {

    Data compute(Context& context, std::shared_ptr<Parser::Expression> caller, Reference const& reference) {
        if (auto symbol = std::get_if<SymbolReference>(&reference))
            if (*symbol == std::get<SymbolReference>(context.get_global()["getter"]))
                return symbol->it->first;

        if (auto d = std::get_if<Data>(&reference); d && *d != Data{})
            return *d;
        else
            return call_function(context, caller, context.get_global()["getter"], reference).to_data(context, caller);
    }

    Data& IndirectReference::get_data() const {
        if (auto symbol_reference = std::get_if<SymbolReference>(this)) {
            return symbol_reference->it->first;
        } else if (auto property_reference = std::get_if<PropertyReference>(this)) {
            auto parent = property_reference->parent;
            if (auto obj = get_if<ObjectPtr>(&parent))
                return (*obj)->properties[property_reference->name];
        } else if (auto array_reference = std::get_if<ArrayReference>(this)) {
            auto array = array_reference->array;
            if (auto obj = get_if<ObjectPtr>(&array))
                return (*obj)->array[array_reference->i];
        }

        static Data empty_data;
        return empty_data = Data{};
    }

    Data IndirectReference::to_data(Context& context, std::shared_ptr<Parser::Expression> caller) const {
        return compute(context, caller, *this);
    }

    Reference::Reference(IndirectReference const& indirect_reference) {
        std::visit([this](auto const& ref) {
            *this = ref;
        }, indirect_reference);
    }

    Data Reference::to_data(Context& context, std::shared_ptr<Parser::Expression> caller) const {
        return compute(context, caller, *this);
    }

    IndirectReference Reference::to_indirect_reference(Context& context, std::shared_ptr<Parser::Expression> caller) const {
        return std::visit(
            overloaded{
                [](Data const& data) -> IndirectReference {
                    return GC::new_reference(data);
                },
                [](SymbolReference const& symbol_reference) -> IndirectReference {
                    return symbol_reference;
                },
                [](PropertyReference const& property_reference) -> IndirectReference {
                    return property_reference;
                },
                [](ArrayReference const& array_reference) -> IndirectReference {
                    return array_reference;
                },
                [&context, &caller](TupleReference const& tuple_reference) -> IndirectReference {
                    auto object = GC::new_object();
                    object->array.reserve(tuple_reference.size());
                    for (auto d : tuple_reference)
                        object->array.push_back(d.to_data(context, caller));
                    return GC::new_reference(Data(object));
                }
            }
        , *this);
    }

}
