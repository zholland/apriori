#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include "hashtable.h"

struct Transaction {
    int *items;
    int numItems;
};

struct Itemset {
    uint32_t *items;
    uint32_t support;
};

struct FrequentItemset {
    size_t size;
    size_t numberOfItemsets;
    struct Itemset *itemsets;
};

// max 28 items on a line

int main(int argc, char *argv[]) {
    clock_t time1 = clock();
    if (argc < 4) {
        printf("Not enough arguments.");
        return EXIT_FAILURE;
    } else if (argc > 5) {
        printf("Too many arguments.");
        return EXIT_FAILURE;
    } else {
        FILE *fp;
        int minSupport = 10000;
        float confidence = 0.5;
        int number;

        int max = 0;
        int min = INT_MAX;
        char inputBuffer[255];
        fp = fopen(argv[1], "r");
//        fp = fopen("/home/zach/CLionProjects/apriori/dataT10K500D12L.data.txt", "r");
//    fp = fopen("/home/zach/CLionProjects/apriori/test.txt", "r");
//    fp = fopen("/Users/zach/CLionProjects/Apriori/test.txt", "r");
        size_t numTransactions = 0;
        size_t maxItemsOnLine = 0;
        size_t numItemsOnLine = 0;
        while (fgets(inputBuffer, 255, fp) != NULL) {
            numTransactions += 1;
            char *p = inputBuffer;
            while (sscanf(p, "%d", &number) == 1) {
                numItemsOnLine += 1;
                if (number > 100) {
                    p += 4;
                } else if (number > 10) {
                    p += 3;
                } else {
                    p += 2;
                }
            }
            if (numItemsOnLine > maxItemsOnLine) {
                maxItemsOnLine = numItemsOnLine;
            }
            numItemsOnLine = 0;
        }

        struct Itemset *Ck = calloc(500, sizeof(struct Itemset));
        struct FrequentItemset *frequentItemsets = calloc(maxItemsOnLine, sizeof(struct FrequentItemset));

        rewind(fp);

        // Initialize C1
        for (uint32_t i = 0; i < 500; i++) {
            Ck[i].items = calloc(1, sizeof(uint32_t));
            Ck[i].items[0] = i;
            Ck[i].support = 0;
        }

        struct Transaction *transactions = calloc(numTransactions, sizeof(struct Transaction));
        int transactionIndex = 0;
        while (fgets(inputBuffer, 255, fp) != NULL) {
            char *p = inputBuffer;
            transactions[transactionIndex].items = calloc(maxItemsOnLine, sizeof(int));
            int itemCount = 0;
            while (sscanf(p, "%d", &number) == 1) {
                Ck[number].support += 1;
                transactions[transactionIndex].items[itemCount] = number;
                itemCount += 1;
                if (number > 100) {
                    p += 4;
                } else if (number > 10) {
                    p += 3;
                } else {
                    p += 2;
                }
            }
            transactions[transactionIndex].numItems = itemCount;
            transactionIndex++;
        }
        fclose(fp);

        size_t numFrequent = 0;
        for (int i = 0; i < 500; i++) {
            if (Ck[i].support > minSupport) {
                numFrequent++;
            }
        }
        frequentItemsets[0].numberOfItemsets = numFrequent;
        frequentItemsets[0].size = 1;
        frequentItemsets[0].itemsets = calloc(numFrequent, sizeof(struct Itemset));
        int itemsetIndex = 0;
        for (int i = 0; i < 500; i++) {
            if (Ck[i].support > minSupport) {
                frequentItemsets[0].itemsets[itemsetIndex].items = Ck[i].items;
                frequentItemsets[0].itemsets[itemsetIndex].support = Ck[i].support;
                itemsetIndex++;
            }
        }

        size_t size = (size_t) numFrequent * numFrequent;

        // Free Ck
//    for (int i = 0; i < 500; i++) {
//        free(Ck[i].items);
//    }
        free(Ck);

        for (size_t k = 0; frequentItemsets[k].numberOfItemsets > 0; k++) {
            // Inititialize Ck
            Ck = calloc(size, sizeof(struct Itemset));

            // Generate candidate itemsets
            int c = 0;
//        Ck[c].items = calloc(k+2, sizeof(uint32_t));
            for (int p = 0; p < frequentItemsets[k].numberOfItemsets; p++) {
                for (int q = p + 1; q < frequentItemsets[k].numberOfItemsets; q++) {
                    struct Itemset itemsetP = frequentItemsets[k].itemsets[p];
                    struct Itemset itemsetQ = frequentItemsets[k].itemsets[q];

                    int i = 0;
                    while (i < k + 1 && itemsetP.items[i] == itemsetQ.items[i]) {
                        i++;
                    }
                    if (i == k) {
                        Ck[c].items = calloc(k + 2, sizeof(uint32_t));
                        int j = 0;
                        while (j < k + 1) {
                            Ck[c].items[j] = itemsetP.items[j];
                            j++;
                        }
                        Ck[c].items[j] = itemsetQ.items[j - 1];
                        c++;
                    }
                }
            }

            /////////////////////////
            //// Prune Ck Step?  ////
            /////////////////////////

            size_t tableSize = 0;
            uint32_t tmp = 1;
            while ((float)c / (float)tmp > 0.7) {
                tableSize++;
                tmp = tmp << 1;
            }

            int numCandidates = c;
            struct Entry *LkTable = create(tableSize);
            for (int t = 0; t < numTransactions; t++) {
                int found = 0;
                for (c = 0; c < numCandidates; c++) {
                    if (Ck[c].items == NULL) {
                        continue;
                    }
                    for (int u = 0; u < transactions[t].numItems; u++) {
                        if (transactions[t].items[u] == Ck[c].items[found]) {
                            found++;
                        }
                    }
                    if (found == k + 2) {
                        incrementCount(LkTable, tableSize, Ck[c].items, k + 2);
                    }
                    found = 0;
                }
            }

            numFrequent = 0;
            for (int i = 0; i < numCandidates; i++) {
                Ck[i].support = getCount(LkTable, tableSize, Ck[i].items, k + 2);
                if (Ck[i].support > minSupport) {
                    numFrequent++;
                }
            }
            frequentItemsets[k + 1].numberOfItemsets = numFrequent;
            frequentItemsets[k + 1].size = k + 2;
            frequentItemsets[k + 1].itemsets = calloc(numFrequent, sizeof(struct Itemset));
            itemsetIndex = 0;
            for (int i = 0; i < numCandidates; i++) {
                if (Ck[i].support > minSupport) {
                    frequentItemsets[k + 1].itemsets[itemsetIndex].items = Ck[i].items;
                    frequentItemsets[k + 1].itemsets[itemsetIndex].support = Ck[i].support;
                    itemsetIndex++;
                }
            }
            free(Ck);
        }
//    size_t key1Size = 3;
//    uint32_t key1[key1Size];
//    key1[0] = 13;
//    key1[1] = 0;
//    key1[2] = 0;
//    incrementCount(fooTable, fooSize, key1, key1Size);
//    incrementCount(fooTable, fooSize, key1, key1Size);
//    incrementCount(fooTable, fooSize, key1, key1Size);
//    size_t key2Size = 3;
//    uint32_t key2[key2Size];
//    key2[0] = 1;
//    key2[1] = 0;
//    key2[2] = 1;
//    incrementCount(fooTable, fooSize, key2, key2Size);
//    size_t key3Size = 3;
//    uint32_t key3[key3Size];
//    key3[0] = 1;
//    key3[1] = 1;
//    key3[2] = 1;
//    incrementCount(fooTable, fooSize, key3, key3Size);
//    incrementCount(fooTable, fooSize, key3, key3Size);
////    uint32_t key1 = 11;
////    uint32_t key2 = 12;
////    incrementCount(fooTable, fooSize, &key1);
////    incrementCount(fooTable, fooSize, &key2);
////    int bar[10];
//    printf("%" PRIu32" \n", getCount(fooTable, fooSize, key1, key1Size));
//    printf("%" PRIu32" \n", getCount(fooTable, fooSize, key2, key2Size));
//    printf("%" PRIu32" \n", getCount(fooTable, fooSize, key3, key3Size));
//    printf("%" PRIu32" \n", getCount(fooTable, fooSize, &key2));
//    printf("%d", );
//    printf("Min %d\n", 10);
//        printf("Max items on line: %zu", maxItemsOnLine);
        printf("Frequent itemsets:\n");
        int k = 0;
        while (frequentItemsets[k].numberOfItemsets > 0) {
            for (int i = 0; i < frequentItemsets[k].numberOfItemsets; i++) {
                printf("{");
                for (int j = 0; j <= k; j++) {
                    printf("%d", frequentItemsets[k].itemsets[i].items[j]);
                    if (j < k) {
                        printf(", ");
                    }
                }
                printf("}\n");
            }
            k++;
        }
        clock_t time2 = clock();
        printf("Elapsed time: %lf s", (double) (time2 - time1) / CLOCKS_PER_SEC);
        return EXIT_SUCCESS;
    }
}