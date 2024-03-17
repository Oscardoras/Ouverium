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

}
