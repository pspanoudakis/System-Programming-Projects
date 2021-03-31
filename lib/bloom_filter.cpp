/**
 * File: bloom_filter.cpp
 * Bloom Filter class implementation.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include "../include/bloom_filter.hpp"
#include "../include/hash_functions.hpp"

/**
 * Create a Bloom Filter with n bytes size.
 */
BloomFilter::BloomFilter(unsigned long n): numBits(n*8), bits(new unsigned char[n])
{
    for (unsigned long i = 0; i < n; i++)
    {
        bits[i] = 0;    // All bytes are initialized to 0.
    }
}

BloomFilter::~BloomFilter()
{
    delete [] bits;
}

/**
 * Set the n-th bit of the Bloom Filter to 1.
 * If this bit does not exist, do nothing.
 */
void BloomFilter::setBit(unsigned long n)
{  
    if (n >= this->numBits) { return; }

    int target_byte = n / (sizeof(unsigned char)*8);    // The number of the byte that contains the specified bit
    int relative_bit = n - target_byte*8;               // The relative number of the bit inside that byte
    unsigned char mask = 1;                             // The mask used to set the bit. Initially only the first bit is 1.
    for (int i = 1; i <= relative_bit; i++)
    // Shifting the mask until the target bit is 1
    {
        mask = mask*2;
    }

    // Using bitwise OR to keep the rest of the bits unchanged
    this->bits[target_byte] = this->bits[target_byte] | mask;
}

/**
 * Returns TRUE if the n-th bit is set to 1, FALSE otherwise.
 * If this bit does not exist, returns FALSE as well.
 */
bool BloomFilter::getBit(unsigned long n)
{
    if (n >= this->numBits) { return false; }

    int target_byte = n / (sizeof(unsigned char)*8);    // The number of the byte that contains the specified bit
    int relative_bit = n - target_byte*8;               // The relative number of the bit inside that byte

    unsigned char temp_byte = this->bits[target_byte];  // Copying the target byte to not change it
    //temp_byte = temp_byte << (8 - relative_bit - 1);  // Proper shifting to set the rest of the bits to 0
    temp_byte = temp_byte << (7 - relative_bit);        // Proper shifting to set the rest of the bits to 0
    temp_byte = temp_byte >> 7;                         // Shift the bit so that it is now the less significant

    // At this point, temp_byte is either 1 or 0, based on the value of the bit.
    return (temp_byte == 1);
}

/**
 * Returns TRUE if all the Bloom Filter bits for the specified data
 * have been set to 1, FALSE otherwise.
 */
bool BloomFilter::isPresent(void *data)
{
    unsigned long bit_num;      // The bit number to check every time
    for (int i = 0; i < K_MAX; i++)
    {
        bit_num = hash_i((unsigned char*)data, i) % this->numBits;
        if ( !getBit(bit_num) ) { return false; }
    }
    return true;
}

/**
 * Sets all the Bloom Filter bits for the specified data to 1.
 */
void BloomFilter::markAsPresent(void *data)
{
    for (int i = 0; i < K_MAX; i++)
    {
        setBit(hash_i((unsigned char*)data, i) % this->numBits);
    }
}