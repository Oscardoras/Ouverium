#include "data.h"


__Array __UnknownData_get_array(__UnknownData data) {
    return data.virtual_table->get_array(data);
}
