#ifndef __VIRTUAL_TABLES_H__
#define __VIRTUAL_TABLES_H__

#include "data.h"


__UnkownData __VirtualTable_int_unknown_data_from(void* ptr);
__VirtualTable __VirtualTable_int {
    .gc_iterator = __UnknownData__get_NULL_array,
    .get_array = __VirtualTable_int_get_array,
    .unknown_data_from = __VirtualTable_int_unknown_data_from;
    .size = sizeof(long);
};

__UnkownData __VirtualTable_float_unknown_data_from(void* ptr);
__VirtualTable __VirtualTable_float {
    .gc_iterator = __UnknownData__get_NULL_array,
    .get_array = __VirtualTable_float_get_array,
    .unknown_data_from = __VirtualTable_float_unknown_data_from;
    .size = sizeof(double);
};

__UnkownData __VirtualTable_char_unknown_data_from(void* ptr);
__VirtualTable __VirtualTable_char {
    .gc_iterator = __UnknownData__get_NULL_array,
    .get_array = __VirtualTable_char_get_array,
    .unknown_data_from = __VirtualTable_char_unknown_data_from;
    .size = sizeof(char);
};

__UnkownData __VirtualTable_bool_unknown_data_from(void* ptr);
__VirtualTable __VirtualTable_bool {
    .gc_iterator = __UnknownData__get_NULL_array,
    .get_array = __VirtualTable_bool_get_array,
    .unknown_data_from = __VirtualTable_bool_unknown_data_from;
    .size = sizeof(bool);
};


#endif
