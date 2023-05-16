#ifndef __REFERENCE_H__
#define __REFERENCE_H__

#include "array.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * References an UnknownData.
 * It belongs to the owner of this object to free the reference with __Reference_free when it is no longer used or to give it to another owner.
*/
typedef struct __Reference_Owned_t {}* __Reference_Owned;

/**
 * References an UnknownData.
 * It does NOT belong to the owner of this object to free the reference.
*/
typedef struct __Reference_Shared_t {}* __Reference_Shared;

/**
 * Creates a new data reference.
 * @param data the UnknownData to reference.
 * @return an owned reference.
*/
__Reference_Owned __Reference_new_data(__UnknownData data);

/**
 * Creates a new symbol reference ie. a reference to a variable containing a data.
 * @return an owned reference.
*/
__Reference_Owned __Reference_new_symbol();

/**
 * Creates a new property reference ie. a reference to the property of an object and that contains a data.
 * @param parent the object in which the property is.
 * @param virtual_table a virtual table of the property type.
 * @param property a pointer to the property.
 * @return an owned reference.
*/
__Reference_Owned __Reference_new_property(__UnknownData parent, __VirtualTable* virtual_table, void* property);

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
 * If you have an owned reference, you must share it before.
 * @param reference the shared reference.
 * @param i the index.
 * @return a tuple reference.
*/
__Reference_Owned __Reference_get_element(__Reference_Shared reference, size_t i);

/**
 * Gets the size of a tuple reference or an array.
 * If you have an owned reference, you must share it before.
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
#define __Reference_share(reference) ((__Reference_Shared) reference)

/**
 * Frees a reference when it has been used.
 * @param reference an owned reference.
*/
void __Reference_free(__Reference_Owned reference);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include <array>

class Reference {

protected:

    void* reference;

public:

    Reference(UnknownData const& data):
        reference{__Reference_new_data(data)} {}
    Reference():
        reference{__Reference_new_symbol()} {}
    Reference(UnknownData const& parent, __VirtualTable* const virtual_table, void* const property):
        reference{__Reference_new_property(parent, virtual_table, property)} {}
    Reference(UnknownData const& array, size_t const i):
        reference{__Reference_new_array(array, i)} {}
    Reference(std::initializer_list<Reference> const& list):
        reference{__Reference_new_tuple((__Reference_Shared *) std::data(list), list.size())} {}

    Reference(__Reference_Owned const reference):
        reference{reference} {}

    Reference(Reference const& reference):
        reference{__Reference_copy(reference)} {}
    Reference(Reference && reference):
        reference{reference.reference} {
        reference.reference = nullptr;
    }

    ~Reference() {
        if (reference != nullptr)
            __Reference_free((__Reference_Owned) reference);
    }

    Reference & operator=(Reference const& reference) {
        if (Reference::reference != reference.reference) {
            __Reference_free((__Reference_Owned) Reference::reference);
            Reference::reference = __Reference_copy((__Reference_Shared) reference.reference);
        }

        return *this;
    }
    Reference & operator=(Reference && reference) {
        if (Reference::reference != reference.reference) {
            Reference::reference = reference.reference;
            reference.reference = nullptr;
        }

        return *this;
    }

    operator __Reference_Owned() {
        auto tmp = (__Reference_Owned) reference;
        reference = nullptr;
        return tmp;
    }
    operator __Reference_Shared() const {
        return (__Reference_Shared) reference;
    }

    operator __UnknownData() const {
        return get();
    }
    __UnknownData get() const {
        return __Reference_get((__Reference_Shared) reference);
    }

    size_t size() const {
        return __Reference_get_size((__Reference_Shared) reference);
    }

    bool empty() const {
        return size() == 0;
    }

    Reference operator[](size_t i) const {
        return __Reference_get_element((__Reference_Shared) reference, i);
    }

    class iterator {

        Reference & reference;
        size_t i;

    public:

        iterator(Reference & reference, size_t i):
            reference{reference}, i{i} {}

        size_t operator++() {
            return i++;
        }

        size_t operator++(int) {
            return ++i;
        }

        size_t operator--() {
            return i--;
        }

        size_t operator--(int) {
            return --i;
        }

        bool operator==(iterator const& it) const {
            return it.i == i;
        }

        bool operator!=(iterator const& it) const {
            return !(it == *this);
        }

        Reference operator*() const {
            return reference[i];
        }

    };

    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, size());
    }

};

#endif


#endif
