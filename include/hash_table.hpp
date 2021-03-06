#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include "utils.hpp"

class LinkedList;

class HashTable
{
    private:
        int size;
        LinkedList **buckets;
        CompareFunc compare;
        DestroyFunc destroy;
        
    public:
        HashTable(int num_buckets, CompareFunc comp, DestroyFunc dest);
        ~HashTable();
};

#endif
