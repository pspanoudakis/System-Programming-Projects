#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include "utils.hpp"

class LinkedList;

class HashTable
{
    private:
        unsigned int size;
        LinkedList **buckets;
        CompareFunc compare;
        DestroyFunc destroy;
        HashObjectFunc get_hash_object;
        unsigned int getHashCode(void *data, unsigned int mod);
        
    public:
        HashTable(int num_buckets, CompareFunc comp, DestroyFunc dest);
        ~HashTable();
        bool insert(void *element);
};

#endif
