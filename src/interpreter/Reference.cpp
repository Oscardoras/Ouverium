#include "Interpreter.hpp"


template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

namespace Interpreter {

    Data compute(Context& context, std::shared_ptr<Parser::Expression> caller, Reference const& reference) {
        if (auto symbol = std::get_if<SymbolReference>(&reference))
            if (*symbol == std::get<SymbolReference>(context.get_global()["getter"]))
                return symbol->get();

        if (auto d = std::get_if<Data>(&reference); d && *d != Data{})
            return *d;
        else
            return call_function(context, caller, context.get_global()["getter"], reference).to_data(context, caller);
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
                [&context](Data const& data) -> IndirectReference {
                    return context.new_reference(data);
                },
                [&context](SymbolReference const& symbol_reference) -> IndirectReference {
                    return symbol_reference;
                },
                [&context](PropertyReference const& property_reference) -> IndirectReference {
                    return property_reference;
                },
                [&context](ArrayReference const& array_reference) -> IndirectReference {
                    return array_reference;
                },
                [&context, &caller](TupleReference const& tuple_reference) -> IndirectReference {
                    auto object = context.new_object();
                    object->array.reserve(tuple_reference.size());
                    for (auto d : tuple_reference)
                        object->array.push_back(d.to_data(context, caller));
                    return context.new_reference(Data(object));
                }
            }
        , *this);
    }

}
