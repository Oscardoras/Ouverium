
#include <memory>
#include <variant>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Interpreter {

    namespace {

        template<class... Ts>
        struct overloaded : Ts... { using Ts::operator()...; };

        Data compute(Context& context, std::shared_ptr<Parser::Expression> const& caller, Reference const& reference) {
            static bool inGetter = false;

            struct GetterScope {
                GetterScope() {
                    inGetter = true;
                }

                ~GetterScope() {
                    inGetter = false;
                }
            };

            if (auto const* d = std::get_if<Data>(&reference))
                return *d;

            if (inGetter) {
                auto const& d = reference.get_data();
                if (d != Data{})
                    return d;
            }

            const auto& getter = context.get_global()["getter"];

            if (auto const* symbol = std::get_if<SymbolReference>(&reference))
                if (*symbol == std::get<SymbolReference>(getter))
                    return **symbol;

            GetterScope scoped{};
            auto r = call_function(context, caller, getter, reference);
            inGetter = false;

            return r.to_data(context, caller);
        }

    }

    IndirectReference::IndirectReference(Reference const& reference) {
        std::visit(
            overloaded{
                [this](Data const& data) {
                    *this = GC::new_reference(data);
                },
                [this](SymbolReference const& symbol_reference) {
                    *this = symbol_reference;
                },
                [this](PropertyReference const& property_reference) {
                    *this = property_reference;
                },
                [this](ArrayReference const& array_reference) {
                    *this = array_reference;
                }
            }
        , reference);
    }

    Data& IndirectReference::get_data() const {
        if (auto const* symbol_reference = std::get_if<SymbolReference>(this)) {
            return **symbol_reference;
        } else if (auto const* property_reference = std::get_if<PropertyReference>(this)) {
            auto parent = property_reference->parent;
            if (auto const* obj = get_if<ObjectPtr>(&parent))
                return (*obj)->properties[property_reference->name];
        } else if (auto const* array_reference = std::get_if<ArrayReference>(this)) {
            auto array = array_reference->array;
            if (auto const* obj = get_if<ObjectPtr>(&array))
                if (array_reference->i < (*obj)->array.size())
                    return (*obj)->array[array_reference->i];
        }

        static Data empty_data;
        return empty_data = Data{};
    }

    Data IndirectReference::to_data(Context& context, std::shared_ptr<Parser::Expression> const& caller) const {
        return compute(context, caller, *this);
    }

    Reference::Reference(IndirectReference const& indirect_reference) {
        std::visit([this](auto const& ref) {
            *this = ref;
        }, indirect_reference);
    }

    Data const& Reference::get_data() const {
        if (auto const* data = std::get_if<Data>(this)) {
            return *data;
        } else if (auto const* symbol_reference = std::get_if<SymbolReference>(this)) {
            return **symbol_reference;
        } else if (auto const* property_reference = std::get_if<PropertyReference>(this)) {
            auto parent = property_reference->parent;
            if (auto const* obj = get_if<ObjectPtr>(&parent))
                return (*obj)->properties[property_reference->name];
        } else if (auto const* array_reference = std::get_if<ArrayReference>(this)) {
            auto array = array_reference->array;
            if (auto const* obj = get_if<ObjectPtr>(&array))
                if (array_reference->i < (*obj)->array.size())
                    return (*obj)->array[array_reference->i];
        }

        static Data empty_data;
        return empty_data = Data{};
    }

    Data Reference::to_data(Context& context, std::shared_ptr<Parser::Expression> const& caller) const {
        return compute(context, caller, *this);
    }

}
