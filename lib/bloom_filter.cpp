#include "../include/bloom_filter.hpp"

BloomFilter::BloomFilter(unsigned int n):bits(new unsigned char[n]), numBits(n*8)
{
    for (int i = 0; i < n; i++)
    {
        bits[i] = 0;
    }
}

BloomFilter::~BloomFilter()
{
    delete bits;
}

void BloomFilter::setBit(unsigned int n)
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

bool BloomFilter::getBit(unsigned int n)
{
    if (n >= this->numBits) { return false; }

    int target_byte = n / (sizeof(unsigned char)*8);
    int relative_bit = n - target_byte*8;

    unsigned char temp_byte = this->bits[target_byte];
    temp_byte = temp_byte << (8 - relative_bit - 1);
    temp_byte = temp_byte >> 7;

    return (temp_byte == 1);
}