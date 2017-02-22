/*
 * An implementation of the Apriori algorithm
 * Author: Zach Holland
 */

#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/*
 * Define structures
 */

// Represents a transaction
struct Transaction {
    int *items;
    int numItems;
};

// Represents an itemset
struct Itemset {
    uint32_t *items;
    uint32_t support;
    size_t size;
};

// Represents a collection of frequent itemsets of a certain size.
struct FrequentItemset {
    size_t size;
    size_t numberOfItemsets;
    struct Itemset *itemsets;
};

// Represents an internal or external node of a hash tree.
struct Node {
    bool isLeaf;
    struct Node **hashtable;
    struct Itemset **itemsets;
    size_t numItemsets;
    struct Node *nextLeaf;
    struct Node *prevLeaf;
};

// Represents a hash tree data structure.
struct HashTree {
    struct Node *root;
    struct Node *firstLeaf;
    size_t maxListSize;
};

/*
 * Define helper functions that are used by the algorithm
 */

/**
 * Creates a hash tree for use with the Apriori algorithm.
 * The internal nodes will have hash tables that are the given size.
 * @param size - The size of the hash tables to create in the internal nodes.
 * @return a pointer to a hash tree of the given size.
 */
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

/**
 * Inserts an itemset into the given hash tree.
 * @param tree - The hash tree to intern the itemset into.
 * @param node - The current node of the hash tree (call with root at the start).
 * @param d  - The current depth of the call to insert in the hash tree (call with 1 at the start).
 * @param itemset - The itemset to insert.
 */
void insert(struct HashTree *tree, struct Node *node, uint32_t d, struct Itemset *itemset) {
    if (node->isLeaf) {
        // If the current node has less itemsets than the maximum, insert the itemset.
        if (node->numItemsets < tree->maxListSize) {
            node->itemsets[node->numItemsets] = itemset;
            node->numItemsets += 1;
        } else {
            // Otherwise we split interior node
            node->isLeaf = false;
            node->hashtable = calloc(tree->maxListSize, sizeof(struct Node *));
            // Initialize the children nodes
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

            // Insert the itemsets from the current node into the lists of the children.
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

/**
 * Counts the number occurences of the itemsets contained in the given transaction.
 * @param tree - The hash tree storing the itemsets.
 * @param node - The current node of the count (call with root at the start).
 * @param transaction - The transaction to count.
 * @param i - The index of the current item in the transaction.
 * @param k - The size of the itemsets to be counted.
 * @param d - The current depth in the hash tree (call with 1 at the start).
 */
void count(struct HashTree *tree, struct Node *node, struct Transaction *transaction, int i, size_t k, int d) {
    if (node->isLeaf) {
        // For each candidate itemset in the node.
        for (int c = 0; c < node->numItemsets; c++) {
            if (node->itemsets != NULL) {
                int j = d - 1;
                int u = i;
                // Compare the current itemset to the transaction.
                while (j < k && u < transaction->numItems) {
                    if (node->itemsets[c]->items[j] == transaction->items[u]) {
                        j++;
                    }
                    u++;
                }
                // If a match was found, increment its support.
                if (j == k) {
                    node->itemsets[c]->support += 1;
                }
            }
        }
    } else {
        // Recursively call count on subsets of the current transaction at the next level of the hash tree.
        while (i <= transaction->numItems - k) {
            count(tree, node->hashtable[transaction->items[i]], transaction, i + 1, k, d + 1);
            i++;
        }
    }
}

/**
 * Gets the support for the given itemset.
 * @param tree - The hash tree containing the itemsets and support counts.
 * @param node - The current node of the tree.
 * @param itemset - The itemset to get the support for.
 * @param d - The current depth of the tree.
 * @param k - The size of the itemset.
 * @return The support count for the given item.
 */
uint32_t getSupport(struct HashTree *tree, struct Node *node, struct Itemset *itemset, int d, size_t k) {
    // If the node is a leaf...
    if (node->numItemsets > 0) {
        // For each candidate itemset in the node.
        for (int c = 0; c < node->numItemsets; c++) {
            if (node->itemsets != NULL) {
                int j = d - 1;
                // Compare the candidate itemsets to the given itemset.
                while (j < itemset->size) {
                    if (node->itemsets[c]->items[j] == itemset->items[j]) {
                        j++;
                    } else {
                        break;
                    }
                }
                // If a match was found.
                if (j == k) {
                    return node->itemsets[c]->support;
                }
            }
        }
        return 0; // The itemset was not found.
    } else {
        // Otherwise, hash to the next level.
        if (node->hashtable != NULL) {
            return getSupport(tree, node->hashtable[itemset->items[d - 1]], itemset, d + 1, k);
        } else {
            return 0; // The itemset was not found.
        }
    }
}

/**
 * Generates the strong association rules from the given itemset with the given confidence.
 * @param C - The hash trees containing the support counts for itemsets of size k > 1
 * @param C1 - The array containing the support counts for itemset of size k = 1
 * @param itemset - The itemset for which to generate rules.
 * @param minConfidence - The minimum confidence of the generated rules.
 * @param numTransactions - The total number of transactions in the dataset.
 * @param print - true if the generated rules should be printed to the screen, false otherwise.
 * @return The number of strong rules generated.
 */
uint32_t generateStrongRules(struct HashTree **C, struct Itemset *C1, struct Itemset *itemset, double minConfidence,
                             size_t numTransactions, bool print) {
    uint32_t numRules = 0;
    size_t numSubsets = (size_t) ((1 << itemset->size) - 1); // The number of possible subsets of the given itemset.
    struct Itemset *antecedent;
    struct Itemset *consequence;

    // For each of the subsets...
    for (uint32_t i = 1; i < numSubsets; i++) {
        antecedent = malloc(sizeof(struct Itemset));
        consequence = malloc(sizeof(struct Itemset));
        antecedent->items = calloc(itemset->size, sizeof(uint32_t));
        consequence->items = calloc(itemset->size, sizeof(uint32_t));
        antecedent->size = 0;
        consequence->size = 0;
        uint32_t a = i;
        uint32_t k = 0;

        // Generate potential antecedents and consequences.
        while (k < itemset->size) {
            if (a & 1) {
                antecedent->items[antecedent->size] = itemset->items[k];
                antecedent->size++;
            } else {
                consequence->items[consequence->size] = itemset->items[k];
                consequence->size++;
            }
            a = a >> 1;
            k++;
        }

        // Get the confidence of the generated rule.
        double confidence;
        if (antecedent->size == 1) {
            confidence = (double) getSupport(C[itemset->size], C[itemset->size]->root, itemset, 1, itemset->size)
                         / (double) C1[antecedent->items[0]].support;
        } else {
            confidence = (double) getSupport(C[itemset->size], C[itemset->size]->root, itemset, 1, itemset->size)
                         / (double) getSupport(C[antecedent->size], C[antecedent->size]->root, antecedent, 1,
                                               antecedent->size);
        }

        if (confidence >= minConfidence) {
            numRules++;
            if (print) {
                // Print the rule, along with its support and confidence.
                int n;
                for (n = 0; n < antecedent->size - 1; n++) {
                    printf("%d, ", antecedent->items[n]);
                }
                printf("%d ", antecedent->items[n]);
                printf("-> ");
                for (n = 0; n < consequence->size - 1; n++) {
                    printf("%d, ", consequence->items[n]);
                }
                printf("%d ", consequence->items[n]);
                printf("(%.2lf,%.2lf)\n", (double) itemset->support / (double) numTransactions, confidence);
            }
        }
    }
    return numRules;
}

/*
 * Define some functions to handle printing
 */

/**
 * Prints the given list of frequent itemsets.
 * @param frequentItemsets - The list of frequent itemsets.
 * @param numTransactions - The total number of transaction in the dataset (for computing the percentage support).
 */
void printFrequentItemsets(struct FrequentItemset *frequentItemsets, size_t numTransactions) {
    int k = 0;
    while (frequentItemsets[k].numberOfItemsets > 0) {
        for (int i = 0; i < frequentItemsets[k].numberOfItemsets; i++) {
            for (int j = 0; j <= k; j++) {
                printf("%d", frequentItemsets[k].itemsets[i].items[j]);
                if (j < k) {
                    printf(" ");
                }
            }
            printf(" (%.2lf)\n", (double) frequentItemsets[k].itemsets[i].support / (double) numTransactions);
//            Uncomment  the following line to print the absolute support, instead of a percentage.
//            printf(" (%d)\n", frequentItemsets[k].itemsets[i].support);
        }
        k++;
    }
}

/**
 * Prints the counts for the number of frequent k_itemsets.
 * @param frequentItemsets - The list of freqent itemsents.
 */
void printFrequentItemsetCounts(struct FrequentItemset *frequentItemsets) {
    int k = 0;
    if (frequentItemsets[0].numberOfItemsets == 0) {
        printf("There are no frequent itemsets with the given support.\n");
    }
    int count = 0;
    while (frequentItemsets[k].numberOfItemsets > 0) {
        count += frequentItemsets[k].numberOfItemsets;
        printf("Number of frequent %d_itemsets: %zu\n", k + 1, frequentItemsets[k].numberOfItemsets);
        k++;
    }
}

/**
 * Prints the list of strong association rules for the given list of freqent itemsests.
 * @param frequentItemsets - The list of frequent itemsets.
 * @param C - The hash trees containing the support counts for itemsets of size k > 1
 * @param C1 - The array containing the support counts for itemset of size k = 1
 * @param minConfidence - The minimum confidence level.
 * @param numTransactions - The total number of transaction in the database.
 */
void printStrongAssociationRules(struct FrequentItemset *frequentItemsets, struct HashTree **C, struct Itemset *C1,
                                 double minConfidence, size_t numTransactions) {
    int k = 1;
    while (frequentItemsets[k].numberOfItemsets > 0) {
        for (int i = 0; i < frequentItemsets[k].numberOfItemsets; i++) {
            generateStrongRules(C, C1, &frequentItemsets[k].itemsets[i], minConfidence, numTransactions, true);
        }
        k++;
    }
}

/**
 * Prints the total number of strong association rules for the given list of frequent itemsests.
 * @param frequentItemsets - The list of frequent itemsets.
 * @param C - The hash trees containing the support counts for itemsets of size k > 1
 * @param C1 - The array containing the support counts for itemset of size k = 1
 * @param minConfidence - The minimum confidence level.
 * @param numTransactions - The total number of transaction in the database.
 */
void printStrongAssociationRuleCount(struct FrequentItemset *frequentItemsets, struct HashTree **C, struct Itemset *C1,
                                     double minConfidence, size_t numTransactions) {
    int k = 1;
    uint32_t ruleCount = 0;
    while (frequentItemsets[k].numberOfItemsets > 0) {
        for (int i = 0; i < frequentItemsets[k].numberOfItemsets; i++) {
            ruleCount += generateStrongRules(C, C1, &frequentItemsets[k].itemsets[i], minConfidence, numTransactions,
                                             false);
        }
        k++;
    }
    printf("Number of association rules: %d\n", ruleCount);
}

/*
 * The main implementation of the Apriori algorithm.
 */

/**
 * Implementation of the Apriori algorithm as first described by Agrawal and Srikant
 * in Fast Algorithms for Mining Association Rules (1994).
 * Takes an input file of transaction and counts the frequent itemsets and generates strong association rules.
 *
 * It takes up to 4 arguments. 1: the name of the file containing the transactions, 2: the minimum support,
 * 3: the minimum confidence, 4: optional modifier ('r', 'f', or 'a'). The support and confidence should be numbers
 * between 0 and 1. The option 'r' prints all the strong association rules; 'f' prints all the frequent itemsets;
 * 'a' prints all the frequent itemsets and strong association rules; when this option is absent only the number of
 * frequent itemsets of different sizes and the number of strong rules are displayed.
 *
 * @param argc - The number of arguments.
 * @param argv - 1: the file containing the transactions, 2: the minimum support,
 *                  3: the minimum confidence, 4: optional modifier ('r', 'f', or 'a')
 * @return 0 if exits successfully, 1 otherwise.
 */
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


        // Try and open the file.
        fp = fopen(argv[1], "r");
        // Check if file was found
        if (fp == NULL) {
            printf("File '%s' was not found.", argv[1]);
            return EXIT_FAILURE;
        }

        size_t maxItemNumber = 0; // The maximum item number read from the file.
        int number; // The current number being read in the transaction.
        char inputBuffer[255];
        size_t numTransactions = 0;
        size_t maxItemsOnLine = 0;
        size_t numItemsOnLine = 0;

        // Read the input file to count the number of transactions, determine the max number of items per tranactions
        // and the maximum item number.
        while (fgets(inputBuffer, 255, fp) != NULL) {
            numTransactions += 1;
            char *p = inputBuffer;
            while (sscanf(p, "%d", &number) == 1) {
                numItemsOnLine += 1;
                if (number >= 100) {
                    p += 4;
                } else if (number >= 10) {
                    p += 3;
                } else {
                    p += 2;
                }
                if (number > maxItemNumber) {
                    maxItemNumber = (size_t) number;
                }
            }
            if (numItemsOnLine > maxItemsOnLine) {
                maxItemsOnLine = numItemsOnLine;
            }
            numItemsOnLine = 0;
        }

        // Initialize an array to hold the candidate itemsets of size 1.
        struct Itemset *C1 = calloc(maxItemNumber + 1, sizeof(struct Itemset));

        // Initialize the frequent itemset list.
        struct FrequentItemset *frequentItemsets = calloc(maxItemsOnLine, sizeof(struct FrequentItemset));

        // Initialize C1
        for (uint32_t i = 0; i <= maxItemNumber; i++) {
            C1[i].items = calloc(1, sizeof(uint32_t));
            C1[i].items[0] = i;
            C1[i].support = 0;
        }

        // Turn the support percentage into an integer value
        int minSupport = (int) (support * numTransactions);

        // Initialize the array to hold the transactions.
        struct Transaction *transactions = calloc(numTransactions, sizeof(struct Transaction));
        int transactionIndex = 0;

        // Go back to the begining of the file and read it again, this time storing the transactions in memory.
        rewind(fp);
        while (fgets(inputBuffer, 255, fp) != NULL) {
            char *p = inputBuffer;
            transactions[transactionIndex].items = calloc(maxItemsOnLine, sizeof(int));
            int itemCount = 0;
            while (sscanf(p, "%d", &number) == 1) {
                // Increment the support of the item and add it to the transaction.
                C1[number].support += 1;
                transactions[transactionIndex].items[itemCount] = number;
                itemCount += 1;
                if (number >= 100) {
                    p += 4;
                } else if (number >= 10) {
                    p += 3;
                } else {
                    p += 2;
                }
            }
            transactions[transactionIndex].numItems = itemCount;
            transactionIndex++;
        }
        fclose(fp);

        // Count the number of 1 frequent itemsets
        size_t numFrequent = 0;
        for (int i = 0; i <= maxItemNumber; i++) {
            if (C1[i].support >= minSupport) {
                numFrequent++;
            }
        }
        // Initialize the 1 frequent itemset list.
        frequentItemsets[0].numberOfItemsets = numFrequent;
        frequentItemsets[0].size = 1;
        frequentItemsets[0].itemsets = calloc(numFrequent, sizeof(struct Itemset));
        // Add the 1 frequent itemsets to the list.
        int itemsetIndex = 0;
        for (int i = 0; i <= maxItemNumber; i++) {
            if (C1[i].support >= minSupport) {
                frequentItemsets[0].itemsets[itemsetIndex].size = 1;
                frequentItemsets[0].itemsets[itemsetIndex].items = calloc(1, sizeof(uint32_t));
                frequentItemsets[0].itemsets[itemsetIndex].items[0] = C1[i].items[0];
                frequentItemsets[0].itemsets[itemsetIndex].support = C1[i].support;
                itemsetIndex++;
            }
        }

        // Create the list of hash trees to hold candidate itemsets of size > 1
        struct HashTree **C = calloc(maxItemsOnLine, sizeof(struct HashTree *));

        // Start of the main loop...
        // Count the number of frequent itemsets of size k > 1.
        for (size_t k = 0; frequentItemsets[k].numberOfItemsets > 0; k++) {
            // Inititialize Ck
            C[k + 2] = createHashTree(maxItemNumber+1);

            // Generate candidate itemsets of size k from the frequent itemsets of size k-1.
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
                        newItemset->size = k + 2;
                        insert(C[k + 2], C[k + 2]->root, 1, newItemset); // Insert the new itemset into the hash tree.
                        c++;
                    }
                }
            }

            // Using the list of transactions, count the support for the candidate itemsets.
            for (int t = 0; t < numTransactions; t++) {
                if (transactions[t].numItems >= k + 2) {
                    count(C[k + 2], C[k + 2]->root, &transactions[t], 0, k + 2, 1);
                }
            }

            // Count the number of frequent k itemsets.
            struct Node *currNode = C[k + 2]->firstLeaf;
            numFrequent = 0;
            while (currNode != NULL) {
                for (int i = 0; i < currNode->numItemsets; i++) {
                    if (currNode->itemsets[i] != NULL && currNode->itemsets[i]->support >= minSupport) {
                        numFrequent++;
                    }
                }
                currNode = currNode->nextLeaf;
            }

            // Initialize the k frequent itemset list.
            frequentItemsets[k + 1].numberOfItemsets = numFrequent;
            frequentItemsets[k + 1].size = k + 2;
            frequentItemsets[k + 1].itemsets = calloc(numFrequent, sizeof(struct Itemset));
            itemsetIndex = 0;

            // Add all the k frequent itemsets to the list.
            currNode = C[k + 2]->firstLeaf;
            while (currNode != NULL) {
                for (int i = 0; i < currNode->numItemsets; i++) {
                    if (currNode->itemsets[i] != NULL && currNode->itemsets[i]->support >= minSupport) {
                        frequentItemsets[k + 1].itemsets[itemsetIndex].size = k + 2;
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
        } // End of the main loop.

        // Print the results depending on the input arguments.
        if (argc == 4) {
            printFrequentItemsetCounts(frequentItemsets);
            printStrongAssociationRuleCount(frequentItemsets, C, C1, confidence, numTransactions);
        } else {
            if (*argv[4] == 'f') {
                printFrequentItemsets(frequentItemsets, numTransactions);
            } else if (*argv[4] == 'r') {
                printStrongAssociationRules(frequentItemsets, C, C1, confidence, numTransactions);
            } else if (*argv[4] == 'a') {
                printFrequentItemsets(frequentItemsets, numTransactions);
                printStrongAssociationRules(frequentItemsets, C, C1, confidence, numTransactions);
            } else {
                printf("Unrecognized parameter: %s\n", argv[4]);
            }
        }
        clock_t time2 = clock();
//        Uncomment to print the elapsed time.
//        printf("Elapsed time: %lf s", (double) (time2 - time1) / CLOCKS_PER_SEC);
        return EXIT_SUCCESS;
    }
}