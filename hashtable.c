#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lookup3.h"

//
// Created by Zach Holland on 2017-01-31.
//

struct Entry {
    uint32_t *key;
    size_t keyLength;
    uint32_t value;
};

uint32_t hashCode(uint32_t *key, size_t keyLength, size_t tableSize) {
    return hashword(key, keyLength, 0) & hashmask(tableSize);
//    return 0;
}

struct Entry *create(size_t tableSize) {
    struct Entry *table = calloc((size_t) (1 << tableSize), sizeof(struct Entry));
    return table;
}

bool keysEqual(uint32_t *key1, uint32_t *key2, size_t keyLength) {
    for (size_t i = 0; i < keyLength; i++) {
        if (key1[i] != key2[i]) {
            return false;
        }
    }
    return true;
}

bool keysEqualOrNull(struct Entry *currEntry, uint32_t *key, size_t keyLength) {
//    struct Entry *currEntry = &table[hashIndex];
    if (currEntry->key == NULL) {
        currEntry->key = key;
        currEntry->keyLength = keyLength;
        currEntry->value = 0;
//        table[hashIndex] = currEntry;
        return true;
    } else if (currEntry->keyLength == keyLength && keysEqual(currEntry->key, key, keyLength)) {
        return true;
    } else {
        return false;
    }
}

struct Entry *getEntry(struct Entry *table, size_t tableSize, uint32_t *key, size_t keyLength) {
    uint32_t hashIndex = hashCode(key, keyLength, tableSize);

    int tableLengthExpanded = 1 << tableSize;
    while (!keysEqualOrNull(&table[hashIndex], key, keyLength)) {
        //go to next cell
        hashIndex += 1;

        //wrap around the table
        hashIndex %= tableLengthExpanded;
    }
    return &table[hashIndex];
}

// Table size is 2^^tableSize
uint32_t incrementCount(struct Entry *table, size_t tableSize, uint32_t *key, size_t keyLength) {
//    uint32_t hashIndex = hashCode(key, keyLength, tableSize);
//
//    int tableLengthExpanded = 1 << tableSize;
//    while (!keysEqualOrNull(table[hashIndex], key, keyLength)) {
//        //go to next cell
//        hashIndex += 1;
//
//        //wrap around the table
//        hashIndex %= tableLengthExpanded;
//    }

    struct Entry *currEntry = getEntry(table, tableSize, key, keyLength);
    currEntry->value += 1;
    return currEntry->value;
}

uint32_t getCount(struct Entry *table, size_t tableSize, uint32_t *key, size_t keyLength) {
    return getEntry(table, tableSize, key, keyLength)->value;
}

void delete(struct Entry *table) {
    free(table);
}

void printValues(struct Entry *table, size_t tableSize) {
    int count = 0;
    for (uint32_t i = 0; i < hashsize(tableSize); i++) {
        if (table[i].value >= 1) {
            count++;
        }
    }
    printf("%d\n", count);
}