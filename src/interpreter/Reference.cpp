#include "Interpreter.hpp"


namespace Interpreter {

    Data compute(Context & context, Reference const& reference, Data const& data) {
        if (data == Data{})
            return call_function(context, context.expression, static_cast<GlobalContext &>(context.get_global()).getter->functions, reference).to_data(context);
        else
            return data;
    }

    PropertyReference::operator Data &() const {
        return parent.get().properties[name];
    }

    ArrayReference::operator Data &() const {
        return array.get().array[i];
    }

    Data IndirectReference::to_data(Context & context) const {
        return compute(context, *this, std::visit([](auto const& arg) -> Data {
            return arg;
        }, *this));
    }

    IndirectReference & IndirectReference::operator=(Data const& data) {
        std::visit([](auto const& arg) -> Data & {
            return arg;
        }, *this) = data;

        return *this;
    }

    IndirectReference & IndirectReference::operator=(Data && data) {
        std::visit([](auto const& arg) -> Data & {
            return arg;
        }, *this) = data;

        return *this;
    }

    Reference::Reference(IndirectReference const& indirect_reference) {
        if (auto symbol_reference = std::get_if<SymbolReference>(&indirect_reference))
            *this = *symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(&indirect_reference))
            *this = *property_reference;
        else if (auto array_reference = std::get_if<ArrayReference>(&indirect_reference))
            *this = *array_reference;
    }

    Data Reference::to_data(Context & context) const {
        auto get_data = [this, &context]() -> Data {
            if (auto data = std::get_if<Data>(this))
                return *data;
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
                    object->array.push_back(d.to_data(context));
                return object;
            } else return Data{};
        };

        return compute(context, *this, get_data());
    }

    IndirectReference Reference::to_indirect_reference(Context & context) const {
        if (auto symbol_reference = std::get_if<SymbolReference>(this))
            return *symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(this))
            return *property_reference;
        else if (auto array_reference = std::get_if<ArrayReference>(this))
            return *array_reference;
        else
            return context.new_reference(to_data(context));
    }

}
