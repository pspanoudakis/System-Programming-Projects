/**
 * File: utils.hpp
 * Contains typedefs/templates widely used by the ADT's and/or the main app.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef UTILS_HPP
#define UTILS_HPP

// Used for 2 element comparison.
// Should return > 0 if a > b, 0 if a == b and < 0 if b > a.
typedef int (*CompareFunc)(void *a, void *b);

// Used for displaying elements.
typedef void (*DisplayFunc)(void *a);

// Used for destroying elements.
typedef void (*DestroyFunc)(void *a);

// Used to return an integer based on the specified element, to get the hashcode of.
typedef int (*HashObjectFunc)(void *a);         

// Used to destroy elements properly (ensuring destructor call & safely casting)
template <class T>
void delete_object(void *object)
{
    delete static_cast<T*>(object);
}

template <class T>
void delete_object_array(void *array)
{
    delete[] static_cast<T*>(array);
}

#endif
