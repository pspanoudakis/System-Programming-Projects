#ifndef UTILS_HPP
#define UTILS_HPP

typedef int (*CompareFunc)(void *a, void *b);
typedef void (*DisplayFunc)(void *a);
typedef void (*DestroyFunc)(void *a);
typedef int (*HashObjectFunc)(void *a);

template <class T>
void delete_object(void *object)
{
    delete (T*)object;
}

#endif
