#include <cstddef>
#include "../include/hash_table.hpp"
#include "../include/linked_list.hpp"

HashTable::HashTable(int num_buckets, CompareFunc comp, DestroyFunc dest):
size(num_buckets), buckets(new LinkedList* [num_buckets]), compare(comp), destroy(dest)
{
    for (int i = 0; i < size; i++)
    {
        buckets[i] = NULL;
    }
}

HashTable::~HashTable()
{
    for (int i = 0; i < size; i++)
    {
        if ( buckets[i] != NULL )
        {
            delete buckets[i];
        }
        delete[] buckets;
    }
}