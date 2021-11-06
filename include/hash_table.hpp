/**
 * File: hash_table.hpp
 * Hash Table definition.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include "utils.hpp"

class LinkedList;

/**
 * A typical Hash Table with seperate chaining.
 */
class HashTable
{
    private:
        unsigned int size;
        LinkedList **buckets;               // The Hash Table is an array of pointers to Linked Lists.
        DestroyFunc destroy;                // Used for destroying elements when the Hash Table is being destroyed.
        HashObjectFunc get_hash_object;     // Returns an integer based on the specified element, to get the hashcode of.
        unsigned int getHashCode(void *data, unsigned int mod) const;
        
    public:
        HashTable(int num_buckets, DestroyFunc dest, HashObjectFunc hash);
        ~HashTable();
        void insert(void *element);
        void* getElement(void *key, CompareFunc compare_func) const;
};

#endif
