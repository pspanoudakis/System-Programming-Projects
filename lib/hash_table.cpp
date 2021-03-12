#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <openssl/sha.h>
#include "../include/hash_table.hpp"
#include "../include/linked_list.hpp"

HashTable::HashTable(int num_buckets, DestroyFunc dest):
size(num_buckets), buckets(new LinkedList* [num_buckets]), destroy(dest)
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

unsigned int HashTable::getHashCode(void *data, unsigned int mod)
{
    int result;

    // Initial hashcode is stored here
    unsigned char *hash = (unsigned char*)malloc(SHA_DIGEST_LENGTH);
    unsigned long int hash_num;

    // Getting the hashcode
    SHA1((unsigned char*)data, sizeof(int), hash);

    // Copying part of it and storing it
    memcpy(&hash_num, hash, sizeof(unsigned int));

    free(hash);

    // Calculating modulo and returning the result in a typical int
    hash_num = hash_num % mod;
    result = hash_num;
    return result;
}

void HashTable::insert(void *element)
{
    int hash_object = this->get_hash_object(element);
    unsigned int hash_code = getHashCode(&hash_object, this->size);

    if (buckets[hash_code] == NULL)
    {
        buckets[hash_code] = new LinkedList(this->destroy);
    }

    buckets[hash_code]->append(element);
}

void* HashTable::getElement(void *key, CompareFunc compare_func)
{
    int hash_object = this->get_hash_object(key);
    unsigned int hash_code = getHashCode(&hash_object, this->size);

    if (buckets[hash_code] == NULL)
    {
        return NULL;
    }
    return buckets[hash_code]->getElement(key, compare_func);
}