#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include "hashtable.h"

struct transaction {
    int items[36];
    int support;
};

// max 28 items on a line

int main() {
    FILE *fp;
    int item;
    int minSupport = 10;
    float confidence = 0.5;
    int number;
    char whitespace;
    int C1[500] = {0};
    int L1[500] = {0};
    int max = 0;
    int min = INT_MAX;
    char inputBuffer[255];
    int count = 0;
    int numTransactions = 0;
    fp = fopen("/Users/zach/CLionProjects/Apriori/dataT10K500D12L.data.txt", "r");
//    fp = fopen("/Users/zach/CLionProjects/Apriori/test.txt", "r");
    int maxItemsOnLine = 0;
    int numItemsOnLine = 0;
    while (fgets(inputBuffer, 255, fp) != NULL) {
        numTransactions += 1;
        char *p = inputBuffer;
        while (sscanf(p, "%d", &number) == 1) {
            C1[number] += 1;

            count += 1;
            if (number > 100) {
                p += 4;
            } else if (number > 10) {
                p += 3;
            } else {
                p += 2;
            }

        }
//        if (numItemsOnLine > maxItemsOnLine) {
//            maxItemsOnLine = numItemsOnLine;
//        }
//        numItemsOnLine = 0;
    }
    fclose(fp);
    int numFrequent = 0;
    for (int i = 0; i < 500; i++) {
        if (C1[i] >= 10) {
            L1[i] = C1[i];
            numFrequent++;
        }
    }
    uint32_t fooSize = 100;
    uint32_t *fooTable = create(fooSize);
    uint32_t key1 = 11;
    uint32_t key2 = 12;
    incrementCount(fooTable, fooSize, &key1);
    incrementCount(fooTable, fooSize, &key2);
    int bar[10];
    printf("%" PRIu32" \n", getCount(fooTable, fooSize, &key1));
    printf("%" PRIu32" \n", getCount(fooTable, fooSize, &key2));
//    printf("%d", );
//    printf("Min %d\n", min);
//    printf("Max items on line: %d", maxItemsOnLine);
    return 0;
}