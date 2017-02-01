//
// Created by Zach Holland on 2017-01-31.
//

#ifndef APRIORI_HASHTABLE_H
#define APRIORI_HASHTABLE_H

#include <stdint.h>

uint32_t * create(uint32_t tableSize);
uint32_t incrementCount(uint32_t *table, uint32_t tableSize, uint32_t *key);
uint32_t getCount(uint32_t *table, uint32_t tableSize, uint32_t *key);
#endif //APRIORI_HASHTABLE_H
