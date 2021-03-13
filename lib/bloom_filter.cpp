#include "../include/bloom_filter.hpp"
#include "../include/hash_functions.hpp"

BloomFilter::BloomFilter(unsigned long n): numBits(n*8), bits(new unsigned char[n])
{
    for (unsigned long i = 0; i < n; i++)
    {
        bits[i] = 0;
    }
}

BloomFilter::~BloomFilter()
{
    delete [] bits;
}

void BloomFilter::setBit(unsigned long n)
{  
    if (n >= this->numBits) { return; }

    int target_byte = n / (sizeof(unsigned char)*8);
    int relative_bit = n - target_byte*8;
    unsigned char mask = 1;
    for (int i = 1; i <= relative_bit; i++)
    {
        mask = mask*2;
    }

    this->bits[target_byte] = this->bits[target_byte] | mask;
}

bool BloomFilter::getBit(unsigned long n)
{
    if (n >= this->numBits) { return false; }

    int target_byte = n / (sizeof(unsigned char)*8);
    int relative_bit = n - target_byte*8;

    unsigned char temp_byte = this->bits[target_byte];
    temp_byte = temp_byte << (8 - relative_bit - 1);
    temp_byte = temp_byte >> 7;

    return (temp_byte == 1);
}

bool BloomFilter::isPresent(void *data)
{
    unsigned long bit_num;
    for (int i = 0; i < K_MAX; i++)
    {
        bit_num = hash_i((unsigned char*)data, i) % this->numBits;
        if ( !getBit(bit_num) ) { return false; }
    }
    return true;
}

void BloomFilter::markAsPresent(void *data)
{
    for (int i = 0; i < K_MAX; i++)
    {
        setBit(hash_i((unsigned char*)data, i) % this->numBits);
    }
}