/**
 * File: hash_functions.cpp
 * Definitions of hash functions used by the Bloom Filter.
 */

#ifndef HASH_FUNC_HPP
#define HASH_FUNC_HPP

unsigned long hash_i(unsigned char *str, unsigned int i);
unsigned long sdbm(unsigned char *str);
unsigned long djb2(unsigned char *str);

#endif
