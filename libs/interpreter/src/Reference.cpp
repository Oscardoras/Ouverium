
#include <memory>
#include <utility>
#include <variant>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Interpreter {

    namespace {

        template<class... Ts>
        struct overloaded : Ts... { using Ts::operator()...; };

        Data compute(Context& context, std::shared_ptr<Parser::Expression> const& caller, Reference const& reference) {
            auto const& getter = context.get_global()["getter"];

            if (auto const* symbol = std::get_if<SymbolReference>(&reference))
                if (*symbol == std::get<SymbolReference>(getter))
                    return **symbol;

            if (auto const* d = std::get_if<Data>(&reference))
                return *d;
            else
                return call_function(context, caller, getter, reference).to_data(context, caller);
        }

    }

    IndirectReference::IndirectReference(Reference reference) {
        std::visit(
            overloaded{
                [this](Data&& data) {
                    *this = GC::new_reference(std::move(data));
                },
                [this](SymbolReference&& symbol_reference) {
                    *this = std::move(symbol_reference);
                },
                [this](PropertyReference&& property_reference) {
                    *this = std::move(property_reference);
                },
                [this](ArrayReference&& array_reference) {
                    *this = std::move(array_reference);
                }
            }
        , std::move(reference));
    }

    Data const& IndirectReference::get_data() const {
        if (auto const* symbol_reference = std::get_if<SymbolReference>(this)) {
            return **symbol_reference;
        } else if (auto const* property_reference = std::get_if<PropertyReference>(this)) {
            auto parent = property_reference->parent;
            if (auto const* obj = get_if<ObjectPtr>(&parent)) {
                auto it = (*obj)->properties.find(property_reference->name);
                if (it != (*obj)->properties.end())
                    return it->second;
            }
        } else if (auto const* array_reference = std::get_if<ArrayReference>(this)) {
            auto array = array_reference->array;
            if (auto const* obj = get_if<ObjectPtr>(&array))
                if (array_reference->i < (*obj)->array.size())
                    return (*obj)->array[array_reference->i];
        }

        static Data empty_data{};
        return empty_data;
    }

    Data& IndirectReference::data() {
        if (auto const* symbol_reference = std::get_if<SymbolReference>(this)) {
            return **symbol_reference;
        } else if (auto const* property_reference = std::get_if<PropertyReference>(this)) {
            auto parent = property_reference->parent;
            if (auto const* obj = get_if<ObjectPtr>(&parent))
                return (*obj)->properties[property_reference->name];
        } else if (auto const* array_reference = std::get_if<ArrayReference>(this)) {
            auto array = array_reference->array;
            if (auto const* obj = get_if<ObjectPtr>(&array))
                return (*obj)->array.at(array_reference->i);
        }

        static Data empty_data{};
        return empty_data;
    }

    Data IndirectReference::to_data(Context& context, std::shared_ptr<Parser::Expression> const& caller) const {
        return compute(context, caller, *this);
    }

    Reference::Reference(IndirectReference indirect_reference) {
        std::visit([this](auto&& ref) {
            *this = std::forward<decltype(ref)>(ref);
        }, std::move(indirect_reference));
    }

    Data const& Reference::get_data() const {
        if (auto const* data = std::get_if<Data>(this)) {
            return *data;
        } else if (auto const* symbol_reference = std::get_if<SymbolReference>(this)) {
            return **symbol_reference;
        } else if (auto const* property_reference = std::get_if<PropertyReference>(this)) {
            auto parent = property_reference->parent;
            if (auto const* obj = get_if<ObjectPtr>(&parent)) {
                auto it = (*obj)->properties.find(property_reference->name);
                if (it != (*obj)->properties.end())
                    return it->second;
            }
        } else if (auto const* array_reference = std::get_if<ArrayReference>(this)) {
            auto array = array_reference->array;
            if (auto const* obj = get_if<ObjectPtr>(&array))
                if (array_reference->i < (*obj)->array.size())
                    return (*obj)->array[array_reference->i];
        }

        static Data empty_data{};
        return empty_data;
    }

    Data Reference::to_data(Context& context, std::shared_ptr<Parser::Expression> const& caller) const {
        return compute(context, caller, *this);
    }

}
