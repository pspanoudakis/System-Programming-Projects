#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#define K_MAX 11

class BloomFilter
{
    private:
        // Using char since it is always 1 byte by standard
        const unsigned long numBits;
        unsigned char *bits;
        void setBit(unsigned long n);
        bool getBit(unsigned long n);
    public:
        BloomFilter(unsigned long n);
        ~BloomFilter();        
        bool isPresent(void *data);
        void markAsPresent(void *data);
};

#endif
