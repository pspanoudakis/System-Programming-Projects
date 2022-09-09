/**
 * File: hash_table.cpp
 * Hash Table implementation.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <openssl/sha.h>
#include "../include/hash_table.hpp"
#include "../include/linked_list.hpp"

/**
 * @brief Create a Hash Table with the specified parameters.
 *
 * @param num_buckets The number of buckets in the Hash Table.
 * @param dest Used for destroying elements when the Hash Table is being destroyed.
 * @param hash A function that returns an integer based on the specified element, to get the hashcode of.
 */
HashTable::HashTable(int num_buckets, DestroyFunc dest, HashObjectFunc hash):
size(num_buckets), buckets(new LinkedList* [num_buckets]), destroy(dest), get_hash_object(hash)
{
    for (unsigned int i = 0; i < size; i++)
    {
        buckets[i] = NULL;
    }
}

HashTable::~HashTable()
{
    for (unsigned int i = 0; i < size; i++)
    {
        if ( buckets[i] != NULL )
        {
            delete buckets[i];
        }
    }
    delete[] buckets;
}

/**
 * Returns the hashcode for the specified data.
 * @param data The data to get the hashcode for.
 * @param mod Used to perform modulo operation on the hashcode.
 */
unsigned int HashTable::getHashCode(void *data, unsigned int mod) const
{
    unsigned int result;

    // Initial hashcode is stored here
    const unsigned char *byte_data = (const unsigned char*)data;
    unsigned char *hash = (unsigned char*)malloc(SHA_DIGEST_LENGTH);
    unsigned long int hash_num;

    // Getting the hashcode
    SHA1(byte_data, sizeof(unsigned int), hash);

    // Copying part of it and storing it
    memcpy(&hash_num, hash, sizeof(unsigned long int));

    free(hash);

    // Calculating modulo and returning the result in a typical int
    hash_num = hash_num % mod;
    result = hash_num;
    return result;
}

/**
 * Inserts the specified element in the Hash Table.
 * No duplicate check takes place.
 */
void HashTable::insert(void *element)
{
    // Get the an int out of the element
    int hash_object = this->get_hash_object(element);
    // Get the hashcode of this int
    unsigned int hash_code = getHashCode(&hash_object, this->size);

    if (buckets[hash_code] == NULL)
    // If the corresponding bucket is empty, create it now.
    {
        buckets[hash_code] = new LinkedList(this->destroy);
    }

    // Insert the element to the bucket in O(1) time.
    buckets[hash_code]->append(element);
}

/**
 * Returns the first element in the Hash Table which is equal to the specified key,
 * according to the specified comparison function.
 * If such element was not found, returns NULL.
 */
void* HashTable::getElement(void *key, CompareFunc compare_func) const
{
    // Get the key hashcode
    unsigned int hash_code = getHashCode(key, this->size);

    if (buckets[hash_code] == NULL)
    // If the corresponding bucket is empty, return NULL
    {
        return NULL;
    }
    // Search the bucket for the element.
    return buckets[hash_code]->getElement(key, compare_func);
}
