/**
 * File: hash_table.hpp
 * Hash Table definition.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include "utils.hpp"

class LinkedList;

class HashTable
{
    private:
        unsigned int size;
        LinkedList **buckets;
        DestroyFunc destroy;
        HashObjectFunc get_hash_object;
        unsigned int getHashCode(void *data, unsigned int mod);
        
    public:
        HashTable(int num_buckets, DestroyFunc dest, HashObjectFunc hash);
        ~HashTable();
        void insert(void *element);
        void* getElement(void *key, CompareFunc compare_func);
};

#endif
