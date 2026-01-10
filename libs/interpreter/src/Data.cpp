#include <any>
#include <cstddef>
#include <string>
#include <typeindex>
#include <utility>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/types.h>


namespace Interpreter {

    Data::Comparators Data::comparators = {
        Data::SimpleComparator<ObjectPtr, ObjectPtr>,
        Data::SimpleComparator<char, char>,
        Data::SimpleComparator<OV_FLOAT, OV_FLOAT>,
        Data::SimpleComparator<OV_INT, OV_INT>,
        Data::SimpleComparator<bool, bool>,
    };

    bool operator==(Data const& a, Data const& b) {
        if (!a.value.has_value() && !b.value.has_value())
            return true;

        auto it = Data::comparators.find({ std::type_index(a.value.type()), std::type_index(b.value.type()) });
        if (it != Data::comparators.end())
            return it->second(a.value, b.value);
        else
            return false;
    }

    PropertyReference Data::get_property(std::string name) {
        return PropertyReference{ .parent = *this, .name = std::move(name) };
    }

    ArrayReference Data::get_at(size_t index) {
        return ArrayReference{ .array = *this, .i = index };
    }

}
