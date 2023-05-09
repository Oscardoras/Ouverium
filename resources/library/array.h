#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "data.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct __Array {
    void *tab;
    size_t size;
    size_t capacity;
} __Array;


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

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

template<typename T>
class Array {

protected:

    __ArrayInfo & array;

public:

    Array(__ArrayInfo & array):
        array{array} {}

    size_t size() const {
        return array.array->size;
    }

    void set_size(size_t size) {
        return __Array_set_size(array, size);
    }

    size_t capacity() const {
        return array.array->capacity;
    }

    void set_capacity(size_t size) {
        return __Array_set_capacity(array, size);
    }

    bool empty() const {
        return size() == 0;
    }

    T & operator[](size_t i) const {
        return *__Array_get(array, i);
    }

    class iterator {

        Array & array;
        size_t i;

    public:

        iterator(Array & array, size_t i):
            array{array}, i{i} {}

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

        T & operator*() const {
            return array[i];
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
