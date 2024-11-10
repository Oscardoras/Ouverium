#include <stdlib.h>

#include <ouverium/include.h>

#include "gc.h"


#define HASHMAP_SIZE 16

Ov_UnknownData Ov_UnknownData_from_data(Ov_VirtualTable* vtable, union Ov_Data d) {
    if (vtable == &Ov_VirtualTable_UnknownData) {
        return *((Ov_UnknownData*) d.ptr);
    } else {
        Ov_UnknownData data = {
            .vtable = vtable,
            .data = d
        };
        return data;
    }
}

Ov_UnknownData Ov_UnknownData_from_ptr(Ov_VirtualTable* vtable, void* ptr) {
    if (vtable == &Ov_VirtualTable_UnknownData)
        return *((Ov_UnknownData*) ptr);
    else {
        Ov_UnknownData data;
        data.vtable = vtable;

        if (vtable == &Ov_VirtualTable_Int)
            data.data.i = *((OV_INT*) ptr);
        else if (vtable == &Ov_VirtualTable_Float)
            data.data.f = *((OV_FLOAT*) ptr);
        else if (vtable == &Ov_VirtualTable_Char)
            data.data.c = *((char*) ptr);
        else if (vtable == &Ov_VirtualTable_Bool)
            data.data.b = *((bool*) ptr);
        else
            data.data.ptr = *((void**) ptr);

        return data;
    }
}

void Ov_UnknownData_set(Ov_VirtualTable* vtable, void* ptr, Ov_UnknownData data) {
    if (vtable == &Ov_VirtualTable_UnknownData)
        *(Ov_UnknownData*) ptr = data;
    else if (vtable == &Ov_VirtualTable_Bool)
        *(bool*) ptr = data.data.b;
    else if (vtable == &Ov_VirtualTable_Char)
        *(char*) ptr = data.data.c;
    else if (vtable == &Ov_VirtualTable_Float)
        *(OV_FLOAT*) ptr = data.data.f;
    else if (vtable == &Ov_VirtualTable_Int)
        *(OV_INT*) ptr = data.data.i;
    else
        *(void**) ptr = data.data.ptr;
}

bool Ov_UnknownData_equals(Ov_UnknownData a, Ov_UnknownData b) {
    if (a.vtable == &Ov_VirtualTable_Char) {
        if (b.vtable == a.vtable)
            return a.data.b == b.data.b;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Float) {
        if (b.vtable == a.vtable)
            return a.data.f == b.data.f;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Int) {
        if (b.vtable == a.vtable)
            return a.data.i == b.data.i;
        else
            return false;
    } else if (a.vtable == &Ov_VirtualTable_Bool) {
        if (b.vtable == a.vtable)
            return a.data.b == b.data.b;
        else
            return false;
    } else {
        return a.data.ptr == b.data.ptr;
    }
}

static Ov_PropertyInfo Ov_UnknownData_get_property_from_map(Ov_UnknownData data, unsigned int hash) {
    if (data.vtable->map.offset < 0) {
        Ov_PropertyInfo property = {
            .vtable = NULL,
            .ptr = NULL
        };
        return property;
    }

    Ov_Map** map = (Ov_Map**) (((BYTE*) data.data.ptr) + data.vtable->map.offset);
    if (*map == NULL)
        *map = calloc(HASHMAP_SIZE, sizeof(struct Ov_Map_Element*));

    struct Ov_Map_Element** map_ptr = &(*map)->tab[hash % (*map)->size];
    while (true) {
        if ((*map_ptr)->hash == hash) {
            Ov_PropertyInfo property = {
                .vtable = &Ov_VirtualTable_UnknownData,
                .ptr = &(*map_ptr)->data
            };
            return property;
        } else if ((*map_ptr)->next == NULL) {
            struct Ov_Map_Element* map_element = malloc(sizeof(struct Ov_Map_Element));
            map_element->hash = hash;
            map_element->data.vtable = NULL;
            map_element->data.data.ptr = NULL;
            *map_ptr = map_element;

            Ov_PropertyInfo property = {
                .vtable = &Ov_VirtualTable_UnknownData,
                .ptr = &(*map_ptr)->data
            };
            return property;
        } else
            map_ptr = &(*map_ptr)->next;
    }
}

Ov_PropertyInfo Ov_UnknownData_get_property(Ov_UnknownData data, unsigned int hash) {
    struct Ov_VirtualTable_Element* list = &data.vtable->table_tab[hash % data.vtable->table_size];
    while (true) {
        if (list->hash == hash) {
            Ov_PropertyInfo property = {
                .vtable = list->vtable,
                .ptr = ((BYTE*) data.data.ptr) + list->offset
            };
            return property;
        } else if (list->next == NULL) {
            return Ov_UnknownData_get_property_from_map(data, hash);
        } else
            list = list->next;
    }
}

Ov_ArrayInfo Ov_UnknownData_get_array(Ov_UnknownData data) {
    Ov_ArrayInfo array = {
        .vtable = data.vtable->array.vtable,
        .array = (Ov_Array*) (((BYTE*) data.data.ptr) + data.vtable->array.offset)
    };
    return array;
}

Ov_Function* Ov_UnknownData_get_function(Ov_UnknownData data) {
    return (Ov_Function*) (((BYTE*) data.data.ptr) + data.vtable->function.offset);
}

void Ov_VirtualTable_Map_gc_iterator(Ov_Map* map) {
    size_t i;
    for (i = 0; i < map->size; ++i) {
        struct Ov_Map_Element* e;
        for (e = map->tab[i]; e != NULL; e = e->next)
            Ov_GC_iterate(Ov_VirtualTable_UnknownData.gc_iterator, &e->data);
    }
}

void Ov_VirtualTable_UnknownData_gc_iterator(void* ptr) {
    Ov_UnknownData data = *((Ov_UnknownData*) ptr);
    if (
        data.vtable != &Ov_VirtualTable_Int &&
        data.vtable != &Ov_VirtualTable_Float &&
        data.vtable != &Ov_VirtualTable_Char &&
        data.vtable != &Ov_VirtualTable_Bool
        )
        Ov_GC_iterate(data.vtable->gc_iterator, data.data.ptr);
}
Ov_VirtualTable Ov_VirtualTable_UnknownData = {
    .size = sizeof(Ov_UnknownData),
    .gc_iterator = Ov_VirtualTable_UnknownData_gc_iterator,
    .array.vtable = NULL,
    .array.offset = -1,
    .function.offset = -1,
    .table_size = 0
};

struct Object {
    Ov_Function Ov_function;
    Ov_Array Ov_array;
    Ov_Map Ov_map;
};
void Ov_VirtualTable_Object_gc_iterator(struct Object* ptr) {
    Ov_GC_iterate((void (*)(void*)) Ov_VirtualTable_Function_gc_iterator, &ptr->Ov_function);
    Ov_GC_iterate((void (*)(void*)) Ov_VirtualTable_Array_gc_iterator, &ptr->Ov_array);
    Ov_GC_iterate((void (*)(void*)) Ov_VirtualTable_Map_gc_iterator, &ptr->Ov_map);
}
Ov_VirtualTable Ov_VirtualTable_Object = {
    .size = sizeof(struct Object),
    .gc_iterator = (void (*)(void*)) Ov_VirtualTable_Object_gc_iterator,
    .array.vtable = &Ov_VirtualTable_UnknownData,
    .array.offset = offsetof(struct Object, Ov_array),
    .function.offset = offsetof(struct Object, Ov_function),
    .map.offset = offsetof(struct Object, Ov_map),
    .table_size = 0
};
Ov_VirtualTable* Ov_VirtualTable_string_from_tuple = &Ov_VirtualTable_Object;

Ov_VirtualTable Ov_VirtualTable_Int = {
    .size = sizeof(OV_INT),
    .gc_iterator = NULL,
    .array.vtable = NULL,
    .array.offset = -1,
    .function.offset = -1,
    .table_size = 0
};

Ov_VirtualTable Ov_VirtualTable_Float = {
    .size = sizeof(OV_FLOAT),
    .gc_iterator = NULL,
    .array.vtable = NULL,
    .array.offset = -1,
    .function.offset = -1,
    .table_size = 0
};

Ov_VirtualTable Ov_VirtualTable_Char = {
    .size = sizeof(char),
    .gc_iterator = NULL,
    .array.vtable = NULL,
    .array.offset = -1,
    .function.offset = -1,
    .table_size = 0
};

Ov_VirtualTable Ov_VirtualTable_Bool = {
    .size = sizeof(bool),
    .gc_iterator = NULL,
    .array.vtable = NULL,
    .array.offset = -1,
    .function.offset = -1,
    .table_size = 0
};
