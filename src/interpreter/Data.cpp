
#include <any>
#include <cstddef>
#include <string>
#include <typeindex>

#include <ouverium/types.h>

#include "Interpreter.hpp"


namespace Interpreter {

    Data::Comparators Data::comparators = {
        Data::SimpleComparator<ObjectPtr, ObjectPtr>,
        Data::SimpleComparator<char, char>,
        Data::SimpleComparator<OV_FLOAT, OV_FLOAT>,
        Data::SimpleComparator<OV_INT, OV_INT>,
        Data::SimpleComparator<bool, bool>,
    };

    bool operator==(Data const& a, Data const& b) {
        if (!static_cast<std::any const&>(a).has_value() && !static_cast<std::any const&>(b).has_value())
            return true;

        auto it = Data::comparators.find({ std::type_index(static_cast<std::any const&>(a).type()), std::type_index(static_cast<std::any const&>(b).type()) });
        if (it != Data::comparators.end())
            return it->second(a, b);
        else
            return false;
    }

    PropertyReference Data::get_property(std::string const& name) {
        return PropertyReference{ .parent = *this, .name = name };
    }

    ArrayReference Data::get_at(size_t index) {
        return ArrayReference{ .array = *this, .i = index };
    }

}
