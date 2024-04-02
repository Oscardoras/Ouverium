#include "../Interpreter.hpp"


template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

namespace Interpreter::SystemFunctions {

    namespace Array {
        void init(GlobalContext&);
    }
    namespace Base {
        void init(GlobalContext&);
    }
    namespace Dll {
        void init(GlobalContext&);
    }
    namespace Math {
        void init(GlobalContext&);
    }
    namespace System {
        void init(GlobalContext&);
    }
    namespace Types {
        void init(GlobalContext&);
    }
    namespace UI {
        void init(GlobalContext&);
    }

    void init(GlobalContext& context) {
        Base::init(context);

        Array::init(context);
        Dll::init(context);
        Math::init(context);
        System::init(context);
        Types::init(context);
        UI::init(context);
    }


    ObjectPtr get_object(GlobalContext& context, IndirectReference const& reference) {
        return std::visit(
            overloaded{
                [&context](SymbolReference const& symbol_reference) {
                    auto& data = symbol_reference.get();
                    if (data == Data{})
                        data = context.new_object();
                    return data.get<ObjectPtr>();
                },
                [&context](PropertyReference const& property_reference) {
                    auto& data = property_reference.parent.get<ObjectPtr>()->properties[property_reference.name];
                    if (data == Data{})
                        data = context.new_object();
                    return data.get<ObjectPtr>();
                },
                [&context](ArrayReference const& array_reference) {
                    auto& data = array_reference.array.get<ObjectPtr>()->array[array_reference.i];
                    if (data == Data{})
                        data = context.new_object();
                    return data.get<ObjectPtr>();
                }
            }
        , reference);
    }

    ObjectPtr get_object(GlobalContext& context, Data& data) {
        if (data == Data{})
            data = context.new_object();
        return data.get<ObjectPtr>();
    }

}
