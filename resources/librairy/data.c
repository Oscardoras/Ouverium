#include "data.h"


__Array __UnknownData__get_NULL_array(__UnknownData data) {
    return __Array {
        .tab = NULL,
        .size = 0,
        .capacity = 0
    };
}

__Array __UnknownData_get_array(__UnknownData data) {
    return data.virtual_table->get_array(data);
}
