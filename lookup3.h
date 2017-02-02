//
// Created by zach on 01/02/17.
//
#include <stdio.h>      /* defines printf for tests */
#include <stdint.h>

#ifndef APRIORI_LOOKUP3_H
#define APRIORI_LOOKUP3_H
#define hashsize(n) ((uint32_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)
uint32_t hashword(
        const uint32_t *k,                   /* the key, an array of uint32_t values */
        size_t          length,               /* the length of the key, in uint32_ts */
        uint32_t        initval);          /* the previous hash, or an arbitrary value */
#endif //APRIORI_LOOKUP3_H
