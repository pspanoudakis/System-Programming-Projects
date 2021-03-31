/**
 * File: bloom_filter.hpp
 * Bloom Filter class definition.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#define K_MAX 11    // Number of Hash Functions used for inserting/detecting data.

/**
 * A simple Bloom Filter, implemented using an array of chars.
 * Unsigned char type is used since it is always 1 byte by standard.
 */
class BloomFilter
{
    private:
        const unsigned long numBits;        // The numer of bits in the Bloom Filter.
        unsigned char *bits;                // The bits of the Bloom Filter (an array of unsigned chars).
        void setBit(unsigned long n);
        bool getBit(unsigned long n);
    public:
        BloomFilter(unsigned long n);
        ~BloomFilter();        
        bool isPresent(void *data);
        void markAsPresent(void *data);
};

#endif
