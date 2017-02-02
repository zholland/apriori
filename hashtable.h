//
// Created by Zach Holland on 2017-01-31.
//

#ifndef APRIORI_HASHTABLE_H
#define APRIORI_HASHTABLE_H

#include <stdint.h>

struct Entry * create(size_t tableSize);
uint32_t incrementCount(struct Entry *table, size_t tableSize, uint32_t *key, size_t keyLength);
uint32_t getCount(struct Entry *table, size_t tableSize, uint32_t *key, size_t keyLength);
void printValues(struct Entry *table, size_t tableSize);
#endif //APRIORI_HASHTABLE_H
