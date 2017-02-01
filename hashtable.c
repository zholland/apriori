#include <stdint.h>
#include <stdlib.h>

//
// Created by Zach Holland on 2017-01-31.
//

struct Entry {
    uint32_t *key;
    uint32_t value;
};

uint32_t hashCode(uint32_t key, uint32_t size) {
    return key % size;
}

struct Entry * create(uint32_t tableSize) {
    struct Entry *table = calloc(tableSize, sizeof(struct Entry));
    return table;
}


uint32_t incrementCount(struct Entry *table, uint32_t tableSize, uint32_t *key) {
    uint32_t hashIndex = hashCode(*key, tableSize);
    struct Entry currEntry;
    currEntry = table[hashIndex];
    if (currEntry.key == NULL) {
        currEntry.key = &key;
        currEntry.value = 1;
    } else if (currEntry.key == key) {

    }
    return 0;
}

uint32_t getCount(struct Entry *table, uint32_t tableSize, uint32_t *key) {
    return table[hashCode(*key, tableSize)].value;
}