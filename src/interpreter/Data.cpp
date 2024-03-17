#include "Interpreter.hpp"


namespace Interpreter {

    Data::Comparators Data::comparators = {
        Data::SimpleComparator<Object*, Object*>,
        Data::SimpleComparator<char, char>,
        Data::SimpleComparator<OV_FLOAT, OV_FLOAT>,
        Data::SimpleComparator<OV_INT, OV_INT>,
        Data::SimpleComparator<bool, bool>,
    };

}
