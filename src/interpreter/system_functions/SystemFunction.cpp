#include "../Interpreter.hpp"


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
        if (auto symbol_reference = std::get_if<SymbolReference>(&reference)) {
            auto& data = symbol_reference->get();
            if (data == Data{})
                data = context.new_object();
            return data.get<ObjectPtr>();
        } else if (auto property_reference = std::get_if<PropertyReference>(&reference)) {
            auto& data = property_reference->parent.get<ObjectPtr>()->properties[property_reference->name];
            if (data == Data{})
                data = context.new_object();
            return data.get<ObjectPtr>();
        } else if (auto array_reference = std::get_if<ArrayReference>(&reference)) {
            auto& data = array_reference->array.get<ObjectPtr>()->array[array_reference->i];
            if (data == Data{})
                data = context.new_object();
            return data.get<ObjectPtr>();
        } else return nullptr;
    }

    ObjectPtr get_object(GlobalContext& context, Data& data) {
        if (data == Data{})
            data = context.new_object();
        return data.get<ObjectPtr>();
    }

}
