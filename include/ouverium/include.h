#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stdbool.h>

#include "types.h"


#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Type of a function iterator called by the garbage collector.
     * This function musts call Ov_GC_iterate on each child element.
    */
    typedef void (*Ov_GC_Iterator)(void*);

    /**
     * Contains information about a data type.
    */
    typedef struct Ov_VirtualTable {
        size_t size;
        Ov_GC_Iterator gc_iterator;
        struct {
            struct Ov_VirtualTable* vtable;
            short offset;
        } array;
        struct {
            short offset;
        } function;
        size_t table_size;
        struct Ov_VirtualTable_Element {
            unsigned int hash;
            struct {
                struct Ov_VirtualTable* vtable;
                short offset;
            };
            struct Ov_VirtualTable_Element* next;
        } table_tab[];
    } Ov_VirtualTable;

    typedef union Ov_Data {
        void* ptr;
        OV_INT i;
        OV_FLOAT f;
        char c;
        bool b;
    } Ov_Data;

    /**
     * Represents a data which the real type is unknown.
    */
    typedef struct Ov_UnknownData {
        Ov_VirtualTable* vtable;
        Ov_Data data;
    } Ov_UnknownData;

    struct Ov_Reference_Owned_t;
    /**
     * References an UnknownData.
     * It belongs to the owner of this object to free the reference with Ov_Reference_free when it is no longer used or to give it to another owner.
    */
    typedef struct Ov_Reference_Owned_t* Ov_Reference_Owned;

    struct Ov_Reference_Shared_t;
    /**
     * References an UnknownData.
     * It does NOT belong to the owner of this object to free the reference.
    */
    typedef struct Ov_Reference_Shared_t* Ov_Reference_Shared;

    /**
     * Represents a property and its virtual table.
    */
    typedef struct Ov_PropertyInfo {
        struct Ov_VirtualTable* vtable;
        void* ptr;
    } Ov_PropertyInfo;

    /**
     * An array component to add in a type.
    */
    typedef struct Ov_Array {
        size_t capacity;
        size_t size;
        void* tab;
    } Ov_Array;

    /**
     * Represents an array and the virtual table of its data.
    */
    typedef struct Ov_ArrayInfo {
        struct Ov_VirtualTable* vtable;
        struct Ov_Array* array;
    } Ov_ArrayInfo;

    typedef bool (*Ov_FunctionFilter)(Ov_Reference_Shared capture[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]);
    typedef Ov_Reference_Owned(*Ov_FunctionBody)(Ov_Reference_Shared capture[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]);

    struct Ov_FunctionCapture {
        enum {
            Ov_FUNCTIONCAPTURE_SYMBOL,
            Ov_FUNCTIONCAPTURE_PROPERTY,
            Ov_FUNCTIONCAPTURE_ARRAY
        } type;
        union {
            Ov_UnknownData* symbol;
            struct {
                Ov_UnknownData parent;
                unsigned int hash;
            } property;
            struct {
                Ov_UnknownData array;
                size_t i;
            } array;
        };
    };

    typedef struct Ov_FunctionCell {
        struct Ov_FunctionCell* next;
        const char* parameters; // r for reference, number for referencing capture, perenthesis for function calls, point then hash for member call
        Ov_FunctionFilter filter;
        Ov_FunctionBody body;
        unsigned short local_variables;
        struct {
            unsigned short size;
            struct Ov_FunctionCapture tab[];
        } captures;
    } Ov_FunctionCell;

    /**
     * Represents a function component to add in a type.
    */
    typedef struct Ov_FunctionCell* Ov_Function;

    typedef struct Ov_Expression {
        enum {
            Ov_EXPRESSION_TUPLE,
            Ov_EXPRESSION_REFERENCE,
            Ov_EXPRESSION_LAMBDA
        } type;
        union {
            struct {
                Ov_VirtualTable* vtable;
                size_t size;
                struct Ov_Expression* tab;
            } tuple;
            Ov_Reference_Shared reference;
            Ov_Function lambda; // is moved to the Ov_Function_eval function.
        };
    } Ov_Expression;


    /**
     * Initialize the environment.
    */
    void Ov_init(void);

    /**
     * Ends the environment.
    */
    void Ov_end(void);

    /**
     * Garbage Collector
    */

    /**
     * Allocates a new object in the memory.
     * @param vtable the vtable of the object.
     * @return a pointer to the object.
    */
    void* Ov_GC_alloc_object(Ov_VirtualTable* vtable);

    /**
     * Iterates an object for the garbage collection.
     * @param iterator the function iterator of the object.
     * @param object a pointer to the object to iterate.
    */
    void Ov_GC_iterate(Ov_GC_Iterator iterator, void* object);

    /**
     * Performs a garbage collection.
    */
    void Ov_GC_collect(void);

    /**
     * Array
    */

    /**
     * Gets an element in an array.
     * @param array the array.
     * @param i the index of the element to get.
     * @return a pointer to the element.
    */
    void* Ov_Array_get(Ov_ArrayInfo array, size_t i);

    /**
     * Sets the size of the array, changes the capacity if necessary.
     * @param array the array.
     * @param size the new size.
    */
    void Ov_Array_set_size(Ov_ArrayInfo array, size_t size);

    /**
     * Sets the capacity of the array.
     * @param array the array.
     * @param capacity the new capacity.
    */
    void Ov_Array_set_capacity(Ov_ArrayInfo array, size_t capacity);

    /**
     * UnknownData
    */

    /**
     * Creates an UnknownData from a virtual table and a data.
     * @param vtable the virtual table of the data.
     * @param d a data.
     * @return an UnknownData.
    */
    Ov_UnknownData Ov_UnknownData_from_data(Ov_VirtualTable* vtable, union Ov_Data d);

    /**
     * Creates an UnknownData from a virtual table and a pointer.
     * @param vtable the virtual table of the data.
     * @param ptr a pointer to the data.
     * @return an UnknownData.
    */
    Ov_UnknownData Ov_UnknownData_from_ptr(Ov_VirtualTable* vtable, void* ptr);

    /**
     * Sets a value from an UnknownData.
     * @param vtable the virtual table of the destination.
     * @param ptr a pointer to the destination.
     * @param data the data to set.
    */
    void Ov_UnknownData_set(Ov_VirtualTable* vtable, void* ptr, Ov_UnknownData data);

    /**
     * Checks if two UnknownData are equals.
     * @param a the first UnknownData.
     * @param b the second UnknownData.
     * @returns if a equals b.
    */
    bool Ov_UnknownData_equals(Ov_UnknownData a, Ov_UnknownData b);

    /**
     * Gets a property from an UnknownData.
     * @param data the UnknownData.
     * @param hash the hash of the property.
     * @return a PropertyInfo representing the property.
    */
    Ov_PropertyInfo Ov_UnknownData_get_property(Ov_UnknownData data, unsigned int hash);

    /**
     * Gets the array of an UnknownData.
     * @param data an UnknownData.
     * @return an ArrayInfo representing the array of the data.
    */
    Ov_ArrayInfo Ov_UnknownData_get_array(Ov_UnknownData data);

    /**
     * Gets the function of an UnknownData.
     * @param data an UnknownData.
     * @return a Function.
    */
    Ov_Function* Ov_UnknownData_get_function(Ov_UnknownData data);

    /**
     * Reference
    */

    /**
     * Creates a new uninitialized data reference.
     * @return an owned reference.
    */
    Ov_Reference_Owned Ov_Reference_new_uninitialized();

    /**
     * Creates a new data reference.
     * @param data the UnknownData to reference.
     * @return an owned reference.
    */
    Ov_Reference_Owned Ov_Reference_new_data(Ov_UnknownData data);

    /**
     * Creates a new symbol reference ie. a reference to a variable containing a data.
     * @param data the UnknownData to reference.
     * @return an owned reference.
    */
    Ov_Reference_Owned Ov_Reference_new_symbol(Ov_UnknownData data);

    /**
     * Creates a new property reference ie. a reference to the property of an object and that contains a data.
     * @param parent the object in which the property is.
     * @param hash the hash of the property.
     * @return an owned reference.
    */
    Ov_Reference_Owned Ov_Reference_new_property(Ov_UnknownData parent, unsigned int hash);

    /**
     * Creates a new array reference ie. a reference to a data inside an array.
     * @param array the array.
     * @param i the index of the element in the array.
     * @return an owned reference.
    */
    Ov_Reference_Owned Ov_Reference_new_array(Ov_UnknownData array, size_t i);

    /**
     * Creates a new tuple reference ie. a list of references.
     * @param references shared references that will be copied in the tuple.
     * @param size the size of the tuple.
     * @param vtable the virtual table if need to create an object from this tuple.
     * @return an owned reference.
    */
    Ov_Reference_Owned Ov_Reference_new_tuple(Ov_Reference_Shared references[], size_t references_size, Ov_VirtualTable* vtable);

    /**
     * Creates a new tuple reference from a string.
     * @param string a string.
     * @param vtable the virtual table if need to create an object from this tuple.
     * @return an owned reference.
    */
    Ov_Reference_Owned Ov_Reference_new_string(const char* string, Ov_VirtualTable* vtable);

    /**
     * Gets the raw UnknownData referenced by a reference, no matter what type of reference.
     * @param reference the shared reference.
     * @return the UnknownData stored in reference, NULL if there is no data.
    */
    Ov_UnknownData Ov_Reference_raw(Ov_Reference_Shared reference);

    /**
     * Gets the UnknownData referenced by a reference, no matter what type of reference.
     * @param reference the shared reference.
     * @return the UnknownData stored in reference.
    */
    Ov_UnknownData Ov_Reference_get(Ov_Reference_Shared reference);

    /**
     * Gets a reference to an element into a tuple reference or an array.
     * @param reference the shared reference.
     * @param i the index.
     * @return a tuple reference.
    */
    Ov_Reference_Owned Ov_Reference_get_element(Ov_Reference_Shared reference, size_t i);

    /**
     * Gets the size of a tuple reference or an array.
     * @param reference the shared reference.
     * @return the size.
    */
    size_t Ov_Reference_get_size(Ov_Reference_Shared reference);

    /**
     * Copies a reference.
     * If you have an owned reference, you must share it before.
     * @param reference the shared reference.
     * @return a Reference.
    */
    Ov_Reference_Owned Ov_Reference_copy(Ov_Reference_Shared reference);

    /**
     * Shares a reference.
     * @param reference the reference.
     * @return a shared reference.
    */
#define Ov_Reference_share(reference) ((Ov_Reference_Shared) (reference))

    /**
     * Frees a reference when it has been used.
     * @param reference an owned reference.
    */
    void Ov_Reference_free(Ov_Reference_Owned reference);

    /**
     * Function
    */

    /**
     * Creates a new function.
     * @return an empty function.
    */
    Ov_Function Ov_Function_new();

    /**
     * Copies a function.
     * @param function the function to copy.
     * @return a copy of the function.
    */
    Ov_Function Ov_Function_copy(Ov_Function function);

    /**
     * Pushes a new implementation in a Function.
     * @param function a pointer to the Function.
     * @param parameters the format of the parameters.
     * @param body the body of the function.
     * @param filter the filter of the function.
     * @param local_variables the number of local variables.
     * @param captures a array of the captures used by the function.
     * @param captures_size the number of captures.
    */
    void Ov_Function_push(Ov_Function* function, const char* parameters, Ov_FunctionBody body, Ov_FunctionFilter filter, size_t local_variables, Ov_Reference_Shared captures[], size_t captures_size);

    /**
     * Pops the last implementation from a Function.
     * @param function a pointer to the Function.
    */
    void Ov_Function_pop(Ov_Function* function);

    /**
     * Removes all the isntances from a Function.
     * @param function a pointer to the Function.
    */
    void Ov_Function_free(Ov_Function* function);

    typedef struct Ov_TryEvalResult {
        bool correct;
        Ov_Reference_Owned reference;
    } Ov_TryEvalResult;

    /**
     * Tries to evaluate a Function.
     * @param function a pointer to the function to evaluate.
     * @param args the arguments to give to the function.
     * @return the return reference or the reference to an exception.
    */
    Ov_TryEvalResult Ov_Function_try_eval(Ov_Function* function, Ov_Expression args);

    /**
     * Evaluates a Function.
     * @param function a pointer to the function to evaluate.
     * @param args the arguments to give to the function.
     * @return the return reference.
    */
    Ov_Reference_Owned Ov_Function_eval(Ov_Function* function, Ov_Expression args);


    /**
     * General
    */

    /**
     * Sets data into a reference by using the setter.
     * @param var a reference.
     * @param data a data reference.
     * @return a reference to the assigned variable.
    */
    Ov_Reference_Owned Ov_set(Ov_Reference_Shared var, Ov_Reference_Shared data);


    extern Ov_VirtualTable Ov_VirtualTable_UnknownData;
    extern Ov_VirtualTable Ov_VirtualTable_Object;
    extern Ov_VirtualTable Ov_VirtualTable_Function;
    extern Ov_VirtualTable Ov_VirtualTable_Int;
    extern Ov_VirtualTable Ov_VirtualTable_Float;
    extern Ov_VirtualTable Ov_VirtualTable_Char;
    extern Ov_VirtualTable Ov_VirtualTable_Bool;


    extern Ov_Reference_Owned getter;
    extern Ov_Reference_Owned function_getter;
    extern Ov_Reference_Owned defined;
    extern Ov_Reference_Owned setter;
    extern Ov_Reference_Owned _x3A_x3D;
    extern Ov_Reference_Owned _x3B;
    extern Ov_Reference_Owned if_statement;
    extern Ov_Reference_Owned else_statement;
    extern Ov_Reference_Owned while_statement;
    extern Ov_Reference_Owned _x24;
    extern Ov_Reference_Owned _x24_x3D_x3D;
    extern Ov_Reference_Owned _x3A;
    extern Ov_Reference_Owned _x3D_x3D;
    extern Ov_Reference_Owned _x21_x3D;
    extern Ov_Reference_Owned _x3D_x3D_x3D;
    extern Ov_Reference_Owned _x21_x3D_x3D;
    extern Ov_Reference_Owned string_from;
    extern Ov_Reference_Owned print;

    extern Ov_Reference_Owned _x21;
    extern Ov_Reference_Owned _x26;
    extern Ov_Reference_Owned _x7C;
    extern Ov_Reference_Owned _x2B;
    extern Ov_Reference_Owned _x2D;
    extern Ov_Reference_Owned _x2A;
    extern Ov_Reference_Owned _x2F;
    extern Ov_Reference_Owned _x25;
    extern Ov_Reference_Owned _x3C;
    extern Ov_Reference_Owned _x3E;
    extern Ov_Reference_Owned _x3C_x3D;
    extern Ov_Reference_Owned _x3E_x3D;
    extern Ov_Reference_Owned _x2B_x2B;
    extern Ov_Reference_Owned _x2D_x2D;
    extern Ov_Reference_Owned _x3A_x2B_x3D;
    extern Ov_Reference_Owned _x3A_x2D_x3D;
    extern Ov_Reference_Owned _x3A_x2A_x3D;
    extern Ov_Reference_Owned _x3A_x2F_x3D;

#ifdef __cplusplus
}
#endif


#endif
