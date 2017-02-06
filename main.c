#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "hashtable.h"

struct Transaction {
    int *items;
    int numItems;
};

struct Itemset {
    uint32_t *items;
    uint32_t support;
    size_t size;
};

struct Subset {
    struct Itemset *itemset;
    struct Subset *nextSubset;
};

struct FrequentItemset {
    size_t size;
    size_t numberOfItemsets;
    struct Itemset *itemsets;
};

struct Node {
    bool isLeaf;
    struct Node **hashtable;
    struct Itemset **itemsets;
    size_t numItemsets;
    struct Node *nextLeaf;
    struct Node *prevLeaf;
};

struct HashTree {
    struct Node *root;
    struct Node *firstLeaf;
    size_t maxListSize;
};

struct Rule {
    bool isFirst;
    uint32_t *anticedent;
    size_t anticedentSize;
    uint32_t *consequence;
    size_t consequenceSize;
    double support;
    double confidence;
    struct Rule *nextRule;
};

// max 28 items on a line

struct HashTree *createHashTree(size_t size) {
    struct Node *rootNode = malloc(sizeof(struct Node));
    rootNode->isLeaf = true;
    rootNode->hashtable = NULL;
    rootNode->itemsets = calloc(size, sizeof(struct Itemset *));
    rootNode->numItemsets = 0;
    rootNode->nextLeaf = NULL;
    rootNode->prevLeaf = NULL;
    struct HashTree *tree = malloc(sizeof(struct HashTree));
    tree->root = rootNode;
    tree->firstLeaf = rootNode;
    tree->maxListSize = size;
    return tree;
}

void insert(struct HashTree *tree, struct Node *node, uint32_t d, struct Itemset *itemset) {
    if (node->isLeaf) {
        if (node->numItemsets < tree->maxListSize) {
            node->itemsets[node->numItemsets] = itemset;
            node->numItemsets += 1;
        } else {
            // Split interior node
            node->isLeaf = false;
            node->hashtable = calloc(tree->maxListSize, sizeof(struct Node *));
            // Init nodes
            struct Node *prevNode = node->prevLeaf;
            for (int i = 0; i < tree->maxListSize; i++) {
                struct Node *currNode = malloc(sizeof(struct Node));;
                node->hashtable[i] = currNode;
                currNode->numItemsets = 0;
                currNode->hashtable = NULL;
                currNode->isLeaf = true;
                currNode->nextLeaf = NULL;
                currNode->itemsets = calloc(tree->maxListSize, sizeof(struct Itemset *));
                currNode->prevLeaf = prevNode;
                if (prevNode != NULL) {
                    prevNode->nextLeaf = currNode;
                } else {
                    tree->firstLeaf = currNode;
                }
                prevNode = currNode;
            }
            prevNode->nextLeaf = node->nextLeaf;
            if (prevNode->nextLeaf != NULL) {
                prevNode->nextLeaf->prevLeaf = prevNode;
            }

            for (int i = 0; i < node->numItemsets; i++) {
                insert(tree, node->hashtable[node->itemsets[i]->items[d - 1]], d + 1, node->itemsets[i]);
            }
            insert(tree, node->hashtable[itemset->items[d - 1]], d + 1, itemset);
            node->itemsets = NULL;
            node->nextLeaf = NULL;
            node->numItemsets = 0;
        }
    } else {
        insert(tree, node->hashtable[itemset->items[d - 1]], d + 1, itemset);
    }
}

void count(struct HashTree *tree, struct Node *node, struct Transaction *transaction, int i, size_t k, int d) {
    if (node->isLeaf) {
        for (int c = 0; c < node->numItemsets; c++) {
            if (node->itemsets != NULL) {
                int j = d - 1;
                int u = i;
                while (j < k && u < transaction->numItems) {
                    if (node->itemsets[c]->items[j] == transaction->items[u]) {
                        j++;
                    }
                    u++;
                }
                if (j == k) {
                    node->itemsets[c]->support += 1;
                }
            }
        }
//        count(tree, node, transaction, i + 1, k, d + 1);
    } else {
        while (i <= transaction->numItems - k) {
            count(tree, node->hashtable[transaction->items[i]], transaction, i + 1, k, d + 1);
            i++;
        }
    }
}

bool isFrequent(struct FrequentItemset *frequentItemsets, struct Itemset *itemset, size_t k) {
    // Iterate through frequent itemsets
    for (int i = 0; i < frequentItemsets[k-1].numberOfItemsets; i++) {
        // Iterate through the elements of the given itemset.
        int j;
        for (j = 0; j < k-1; j++) {
            if (frequentItemsets[k-1].itemsets[i].items[j] != itemset->items[j]) {
                break;
            }
        }
        if (j == k-1 && itemset->items[j] == frequentItemsets[k-1].itemsets[i].items[j]) {
            return true;
        }
    }
    return false;
}

bool isPotentiallyFrequent(struct FrequentItemset *frequentItemsets, struct Itemset *itemset, size_t k) {
    struct Itemset *subset = malloc(sizeof(struct Itemset));
    subset->items = calloc(k, sizeof(uint32_t));
    for (int i = 0; i < k; i++) {
        int p = 0;
        for (int j = 0; j < k; j++) {
            if (p == i) {
                p++;
            }
            subset->items[j] = itemset->items[p];
            p++;
        }
        bool frequent = isFrequent(frequentItemsets, subset, k);
        if (!frequent) {
            free(subset->items);
            free(subset);
            return false;
        }
    }
    free(subset->items);
    free(subset);
    return true;
}

void printFrequentItemsets(struct FrequentItemset *frequentItemsets) {
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
            printf("} (%d)\n", frequentItemsets[k].itemsets[i].support);
        }
        k++;
    }
}

void printFrequentItemsetCounts(struct FrequentItemset *frequentItemsets) {
    int k = 0;
    if (frequentItemsets[0].numberOfItemsets == 0) {
        printf("There are no frequent itemsets with the given support.\n");
    }
    while (frequentItemsets[k].numberOfItemsets > 0) {
        printf("Number of frequent %d_itemsets: %zu\n", k+1, frequentItemsets[k].numberOfItemsets);
        k++;
    }
}

struct Itemset ** generateNonEmptySubsets(struct Itemset *itemset) {
    size_t numSubsets = (size_t)((1 << itemset->size) - 1);
    struct Itemset *newItemset;
    struct Itemset **subsets = calloc(numSubsets, sizeof(struct Itemset *));
    for (uint32_t i = 1; i < numSubsets; i++) {
        newItemset = malloc(sizeof(struct Itemset));
        newItemset->items = calloc(itemset->size, sizeof(uint32_t));
        newItemset->size = 0;
        uint32_t j = i;
        uint32_t k = 0;
        while (j > 0) {
            if (j & 1) {
                newItemset->items[newItemset->size] = itemset->items[k];
//                printf("%d ", newItemset->items[j-1]);
                printf("%d ", itemset->items[k]);
                newItemset->size++;
            }
            j = j >> 1;
            k++;
        }
        printf("\n");
        subsets[i-1] = newItemset;
    }
    return subsets;
}

struct Rule * generateStrongRules(struct Rule *currRule, struct Itemset itemset) {
    return NULL;
}

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
        double support = strtod(argv[2], NULL);
        double confidence = strtod(argv[3], NULL);
        int number;

        int max = 0;
        int min = INT_MAX;
        char inputBuffer[255];
        fp = fopen(argv[1], "r");
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

        struct Itemset *C1 = calloc(500, sizeof(struct Itemset));
        struct FrequentItemset *frequentItemsets = calloc(maxItemsOnLine, sizeof(struct FrequentItemset));
        struct HashTree **candidateHashTrees = calloc(maxItemsOnLine, sizeof(struct HashTree *));
        candidateHashTrees[0] = createHashTree(500);

        rewind(fp);

        // Initialize C1
        for (uint32_t i = 0; i < 500; i++) {
            C1[i].items = calloc(1, sizeof(uint32_t));
            C1[i].items[0] = i;
            C1[i].support = 0;
        }

        // Turn the support percentage into an integer value
        int minSupport = (int) (support * numTransactions);
        printf("Min number of transactions %d\n", minSupport);
//        int minSupport = 1;

        struct Transaction *transactions = calloc(numTransactions, sizeof(struct Transaction));
        int transactionIndex = 0;
        while (fgets(inputBuffer, 255, fp) != NULL) {
            char *p = inputBuffer;
            transactions[transactionIndex].items = calloc(maxItemsOnLine, sizeof(int));
            int itemCount = 0;
            while (sscanf(p, "%d", &number) == 1) {

                C1[number].support += 1;
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

        // Get 1 frequent itemsets
        size_t numFrequent = 0;
        for (int i = 0; i < 500; i++) {
            if (C1[i].support >= minSupport) {
                numFrequent++;
            }
        }
        frequentItemsets[0].numberOfItemsets = numFrequent;
        frequentItemsets[0].size = 1;
        frequentItemsets[0].itemsets = calloc(numFrequent, sizeof(struct Itemset));
        int itemsetIndex = 0;
        for (int i = 0; i < 500; i++) {
            if (C1[i].support >= minSupport) {
                frequentItemsets[0].itemsets[itemsetIndex].items = calloc(1, sizeof(uint32_t));
                frequentItemsets[0].itemsets[itemsetIndex].items[0] = C1[i].items[0];
//                frequentItemsets[0].itemsets[itemsetIndex].items = Ck[i].items;
                frequentItemsets[0].itemsets[itemsetIndex].support = C1[i].support;
                itemsetIndex++;
            }
//            free(Ck[i].items);
        }

//        printFrequentItemsets(frequentItemsets);


        size_t size = (size_t) numFrequent * numFrequent;

        // Free Ck
//    for (int i = 0; i < 500; i++) {
//        free(Ck[i].items);
//    }
//        free(Ck);

        for (size_t k = 0; frequentItemsets[k].numberOfItemsets > 0; k++) {
            // Inititialize Ck
            struct HashTree *Ck = createHashTree(500);

            // Generate candidate itemsets
            int c = 0;
            for (int p = 0; p < frequentItemsets[k].numberOfItemsets; p++) {
                for (int q = p + 1; q < frequentItemsets[k].numberOfItemsets; q++) {
                    struct Itemset itemsetP = frequentItemsets[k].itemsets[p];
                    struct Itemset itemsetQ = frequentItemsets[k].itemsets[q];

                    int i = 0;
                    while (i < k + 1 && itemsetP.items[i] == itemsetQ.items[i]) {
                        i++;
                    }
                    if (i == k) {
                        struct Itemset *newItemset = malloc(sizeof(struct Itemset));
                        newItemset->items = calloc(k + 2, sizeof(uint32_t));
                        newItemset->support = 0;
                        int j = 0;
                        while (j < k + 1) {
                            newItemset->items[j] = itemsetP.items[j];
                            j++;
                        }
                        newItemset->items[j] = itemsetQ.items[j - 1];
//                        for (int n = 0; n < k+2; n++) {
//                            printf("%d ", newItemset->items[n]);
//                        }
//                        printf("\n");
                        insert(Ck, Ck->root, 1, newItemset);
                        c++;
                    }
                }
            }

            /////////////////////////
            //// Prune Ck Step?  ////
            /////////////////////////

            struct Node *currNode = Ck->firstLeaf;
//            while (currNode != NULL) {
//                for (int i = 0; i < currNode->numItemsets; i++) {
//                    if (!isPotentiallyFrequent(frequentItemsets, currNode->itemsets[i], k+1)) {
//                        // Need to free?
//                        currNode->itemsets[i] = NULL;
//                    }
//                }
//                currNode = currNode->nextLeaf;
//            }

//            size_t tableSize = 0;
//            uint32_t tmp = 1;
//            while ((float) c / (float) tmp > 0.7) {
//                tableSize++;
//                tmp = tmp << 1;
//            }

            for (int t = 0; t < numTransactions; t++) {
                if (transactions[t].numItems >= k+2) {
//                    for (int i = 0; i <= transactions[t].numItems - (k + 2); i++) {
                        count(Ck, Ck->root, &transactions[t], 0, k + 2, 1);
//                    }
                }
            }

            currNode = Ck->firstLeaf;
            numFrequent = 0;
            while (currNode != NULL) {
                for (int i = 0; i < currNode->numItemsets; i++) {
                    if (currNode->itemsets[i] != NULL && currNode->itemsets[i]->support >= minSupport) {
                        numFrequent++;
                    }
                }
                currNode = currNode->nextLeaf;
            }
//            for (int i = 0; i < numCandidates; i++) {
//                Ck[i].support = getCount(LkTable, tableSize, Ck[i].items, k + 2);
//                if (Ck[i].support >= minSupport) {
//                    numFrequent++;
//                }
//            }

            frequentItemsets[k + 1].numberOfItemsets = numFrequent;
            frequentItemsets[k + 1].size = k + 2;
//            printf("Pointer before: %p\n", &frequentItemsets[0].itemsets[0].items[0]);
//            printf("Value before: %d\n", frequentItemsets[0].itemsets[0].items[0]);
            frequentItemsets[k + 1].itemsets = calloc(numFrequent, sizeof(struct Itemset));
//            printf("Pointer after: %p\n", &frequentItemsets[0].itemsets[0].items[0]);
//            printf("Value after: %d\n", frequentItemsets[0].itemsets[0].items[0]);
            itemsetIndex = 0;
            currNode = Ck->firstLeaf;
            while (currNode != NULL) {
                for (int i = 0; i < currNode->numItemsets; i++) {
                    if (currNode->itemsets[i] != NULL && currNode->itemsets[i]->support >= minSupport) {
                        frequentItemsets[k + 1].itemsets[itemsetIndex].items = calloc(k + 2, sizeof(uint32_t));
                        for (int j = 0; j < k + 2; j++) {
                            frequentItemsets[k + 1].itemsets[itemsetIndex].items[j] = currNode->itemsets[i]->items[j];
                        }
                        frequentItemsets[k + 1].itemsets[itemsetIndex].support = currNode->itemsets[i]->support;
                        itemsetIndex++;
                    }
                }
                currNode = currNode->nextLeaf;
            }
        }
        struct Itemset *fooItemset = malloc(sizeof(struct Itemset));
        fooItemset->size = 4;
        fooItemset->support = 0;
        fooItemset->items = calloc(4, sizeof(uint32_t));
        fooItemset->items[0] = 1;
        fooItemset->items[1] = 2;
        fooItemset->items[2] = 3;
        fooItemset->items[3] = 4;
        struct Itemset **subsets = generateNonEmptySubsets(fooItemset);
        for (int i = 0; i < (1 << fooItemset->size) - 2; i++) {
            for (int j=0; j < subsets[i]->size; j++) {
                printf("%d ", subsets[i]->items[j]);
            }
            printf("\n");
        }

//        do {
//            for (int i = 0; i < subset->itemset->size; i++) {
//                printf("i=%d: %d ", i, subset->itemset->items[i]);
//            }
//            printf("\n");
//            subset = subset->nextSubset;
//        } while (subset->nextSubset != NULL);
//        // Generate strong rules
//        struct Rule *firstRule = malloc(sizeof(struct Rule));
//        firstRule->isFirst = true;
//        firstRule->anticedentSize = 0;
//        struct Rule *currRule = firstRule;
//        int k = 0;
//        while (frequentItemsets[k].numberOfItemsets > 0) {
//            for (int i = 0; i < frequentItemsets[k].numberOfItemsets; i++) {
//                currRule = generateStrongRules(currRule, frequentItemsets->itemsets[i]);
//            }
//            k++;
//        }

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
        if (argc == 4) {
            printFrequentItemsetCounts(frequentItemsets);
        } else {
            if (*argv[4] == 'f') {
                printFrequentItemsets(frequentItemsets);
            } else if (*argv[4] == 'r') {
                printf("r\n");
            } else if (*argv[4] == 'a') {
                printf("a\n");
            } else {
                printf("Unrecognized parameter: %s\n", argv[4]);
            }
        }
        clock_t time2 = clock();
        printf("Elapsed time: %lf s", (double) (time2 - time1) / CLOCKS_PER_SEC);
        return EXIT_SUCCESS;
    }
}