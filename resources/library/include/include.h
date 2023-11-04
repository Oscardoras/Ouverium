#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stdbool.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

#define BYTE unsigned char

    /**
     * Type of a function iterator called by the garbage collector.
     * This function musts call __GC_iterate on each child element.
    */
    typedef void (*__GC_Iterator)(void*);

    typedef void (*__GC_Destructor)(void*);

    /**
     * Contains information about a data type.
    */
    typedef struct __VirtualTable {
        size_t size;
        __GC_Iterator gc_iterator;
        __GC_Destructor gc_destructor;
        struct {
            struct __VirtualTable* vtable;
            size_t offset;
        } array;
        struct {
            size_t offset;
        } function;
        struct {
            size_t size;
            struct __VirtualTable_Element {
                unsigned int hash;
                union {
                    size_t offset;
                    void (*ptr)();
                };
                struct __VirtualTable_Element* next;
            }*tab[];
        } table;
    } __VirtualTable;

    typedef union __Data {
        void* ptr;
        long i;
        double f;
        char c;
        bool b;
    } __Data;

    /**
     * Represents a data which the real type is unknown.
    */
    typedef struct __UnknownData {
        __VirtualTable* virtual_table;
        __Data data;
    } __UnknownData;

    /**
     * References an UnknownData.
     * It belongs to the owner of this object to free the reference with __Reference_free when it is no longer used or to give it to another owner.
    */
    typedef struct __Reference_Owned_t {}*__Reference_Owned;

    /**
     * References an UnknownData.
     * It does NOT belong to the owner of this object to free the reference.
    */
    typedef struct __Reference_Shared_t {}*__Reference_Shared;

    /**
     * An array component to add in a type.
    */
    typedef struct __Array {
        size_t capacity;
        size_t size;
        void* tab;
    } __Array;

    /**
     * Represents an array and the virtual table of its data.
    */
    typedef struct __ArrayInfo {
        struct __VirtualTable* vtable;
        struct __Array* array;
    } __ArrayInfo;

    typedef bool (*__FunctionFilter)(__Reference_Owned capture[], __Reference_Shared args[]);
    typedef __Reference_Owned(*__FunctionBody)(__Reference_Owned capture[], __Reference_Shared args[]);

    struct __FunctionCapture {
        enum {
            __FUNCTIONCAPTURE_SYMBOL,
            __FUNCTIONCAPTURE_PROPERTY,
            __FUNCTIONCAPTURE_ARRAY
        } type;
        union {
            __UnknownData* symbol;
            struct {
                __UnknownData parent;
                __VirtualTable* virtual_table;
                unsigned int hash;
            } property;
            struct {
                __UnknownData array;
                size_t i;
            } array;
        };
    };

    typedef struct __FunctionCell {
        struct __FunctionCell* next;
        const char* parameters;
        __FunctionFilter filter;
        __FunctionBody body;
        struct {
            unsigned short size;
            struct __FunctionCapture tab[];
        } captures;
    } __FunctionCell;

    /**
     * Represents a function component to add in a type.
    */
    typedef struct __FunctionCell* __Function;

    typedef struct __Expression {
        enum {
            __EXPRESSION_TUPLE,
            __EXPRESSION_REFERENCE,
            __EXPRESSION_LAMBDA
        } type;
        union {
            struct {
                size_t size;
                struct __Expression* tab;
            } tuple;
            __Reference_Shared reference;
            __Function lambda;
        };
    } __Expression;


    /**
     * Garbage Collector
    */

    /**
     * Initialize the Garbage Collector.
    */
    void __GC_init(void);

    /**
     * Ends the Garbage Collector.
    */
    void __GC_end(void);

    /**
     * Allocates a new object in the memory.
     * @param vtable the vtable of the object.
     * @return a pointer to the object.
    */
    void* __GC_alloc_object(__VirtualTable* vtable);

    /**
     * Iterates an object for the garbage collection.
     * @param iterator the function iterator of the object.
     * @param object a pointer to the object to iterate.
    */
    void __GC_iterate(__GC_Iterator iterator, void* object);

    /**
     * Performs a garbage collection.
    */
    void __GC_collect(void);

    /**
     * Array
    */

    /**
     * Creates a new array.
     * @param vtable the vtable of array elements.
     * @param capacity the initial capacity of the array.
     * @return a created array.
    */
    __Array __Array_new(struct __VirtualTable* vtable, size_t capacity);

    /**
     * Gets an element in an array.
     * @param array the array.
     * @param i the index of the element to get.
     * @return a pointer to the element.
    */
    void* __Array_get(__ArrayInfo array, size_t i);

    /**
     * Sets the size of the array, changes the capacity if necessary.
     * @param array the array.
     * @param size the new size.
    */
    void __Array_set_size(__ArrayInfo array, size_t size);

    /**
     * Sets the capacity of the array.
     * @param array the array.
     * @param capacity the new capacity.
    */
    void __Array_set_capacity(__ArrayInfo array, size_t capacity);

    /**
     * Frees an array.
     * @param array the array to free.
    */
    void __Array_free(__ArrayInfo array);

    /**
     * UnknownData
    */

    /**
     * Creates an UnknownData from a virtual table and a data.
     * @param vtable the virtual table of the data.
     * @param d a data.
     * @return an UnknownData.
    */
    __UnknownData __UnknownData_from_data(__VirtualTable* vtable, union __Data d);

    /**
     * Creates an UnknownData from a virtual table and a pointer.
     * @param vtable the virtual table of the data.
     * @param ptr a pointer to the data.
     * @return an UnknownData.
    */
    __UnknownData __UnknownData_from_ptr(__VirtualTable* vtable, void* ptr);

    /**
     * Gets a property from an UnknownData.
     * @param data the UnknownData.
     * @param hash the hash of the property.
     * @return a pointer to the property.
    */
    void* __UnknownData_get_property(__UnknownData data, unsigned int hash);

    /**
     * Gets the array of an UnknownData.
     * @param data an UnknownData.
     * @return an ArrayInfo representing the array of the data.
    */
    __ArrayInfo __UnknownData_get_array(__UnknownData data);

    /**
     * Gets the function of an UnknownData.
     * @param data an UnknownData.
     * @return a Function.
    */
    __Function* __UnknownData_get_function(__UnknownData data);

    /**
     * Reference
    */

    /**
     * Creates a new data reference.
     * @param data the UnknownData to reference.
     * @return an owned reference.
    */
    __Reference_Owned __Reference_new_data(__UnknownData data);

    /**
     * Creates a new symbol reference ie. a reference to a variable containing a data.
     * @param data the UnknownData to reference.
     * @return an owned reference.
    */
    __Reference_Owned __Reference_new_symbol(__UnknownData data);

    /**
     * Creates a new property reference ie. a reference to the property of an object and that contains a data.
     * @param parent the object in which the property is.
     * @param virtual_table a virtual table of the property type.
     * @param hash the hash of the property.
     * @return an owned reference.
    */
    __Reference_Owned __Reference_new_property(__UnknownData parent, __VirtualTable* virtual_table, unsigned int hash);

    /**
     * Creates a new array reference ie. a reference to a data inside an array.
     * @param array the array.
     * @param i the index of the element in the array.
     * @return an owned reference.
    */
    __Reference_Owned __Reference_new_array(__UnknownData array, size_t i);

    /**
     * Creates a new tuple reference ie. a list of references.
     * @param references shared references that will be copied in the tuple.
     * @param size the size of the tuple.
     * @return an owned reference.
    */
    __Reference_Owned __Reference_new_tuple(__Reference_Shared references[], size_t references_size);

    /**
     * Gets the UnknownData referenced by a reference, no matter what type of reference.
     * @param reference the shared reference.
     * @return the UnknownData stored in reference.
    */
    __UnknownData __Reference_get(__Reference_Shared reference);

    /**
     * Gets a reference to an element into a tuple reference or an array.
     * @param reference the shared reference.
     * @param i the index.
     * @return a tuple reference.
    */
    __Reference_Owned __Reference_get_element(__Reference_Shared reference, size_t i);

    /**
     * Gets the size of a tuple reference or an array.
     * @param reference the shared reference.
     * @return the size.
    */
    size_t __Reference_get_size(__Reference_Shared reference);

    /**
     * Copies a reference.
     * If you have an owned reference, you must share it before.
     * @param reference the shared reference.
     * @return a Reference.
    */
    __Reference_Owned __Reference_copy(__Reference_Shared reference);

    /**
     * Shares a reference.
     * @param reference the reference.
     * @return a shared reference.
    */
#define __Reference_share(reference) ((__Reference_Shared) (reference))

    /**
     * Frees a reference when it has been used.
     * @param reference an owned reference.
    */
    void __Reference_free(__Reference_Owned reference);

    /**
     * Function
    */

    /**
     * Creates a new function.
     * @return an empty function.
    */
    __Function __Function_new();

    /**
     * Copies a function.
     * @param function the function to copy.
     * @return a copy of the function.
    */
    __Function __Function_copy(__Function function);

    /**
     * Pushes a new implementation in a Function.
     * @param function a pointer to the Function.
     * @param parameters the format of the parameters.
     * @param body the body of the function.
     * @param filter the filter of the function.
     * @param references a array of the references used by the function.
     * @param references_size the number of references.
    */
    void __Function_push(__Function* function, const char* parameters, __FunctionBody body, __FunctionFilter filter, __Reference_Owned references[], size_t references_size);

    /**
     * Pops the last implementation from a Function.
     * @param function a pointer to the Function.
    */
    void __Function_pop(__Function* function);

    /**
     * Removes all the isntances from a Function.
     * @param function a pointer to the Function.
    */
    void __Function_free(__Function* function);

    /**
     * Evaluates a Function.
     * @param function a pointer to the function to evaluate.
     * @param args the arguments to give to the function.
     * @return the return reference.
    */
    __Reference_Owned __Function_eval(__Function* function, __Expression args);


    extern __VirtualTable __VirtualTable_UnknownData;
    extern __VirtualTable __VirtualTable_Array;
    extern __VirtualTable __VirtualTable_Function;
    extern __VirtualTable __VirtualTable_Int;
    extern __VirtualTable __VirtualTable_Float;
    extern __VirtualTable __VirtualTable_Char;
    extern __VirtualTable __VirtualTable_Bool;

#ifdef __cplusplus
}
#endif


#endif
