#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

class BloomFilter
{
    private:
        // Using char since it is always 1 byte by standard
        unsigned char *bits;
    public:
        BloomFilter(unsigned int n);
        ~BloomFilter();
        void setBit(unsigned int n);
        bool getBit(unsigned int n);
        const unsigned int numBits;
};

#endif
