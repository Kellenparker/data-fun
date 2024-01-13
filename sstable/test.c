#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "memtable.h"
#include "sstable.h"
#include "test.h"

/*
 * ######################
 * Memtree test functions
 * ######################
 */

/*
 * void testMemtableInsertAndSearch(int iterations)
 *   Tests the memtable by inserting and searching for nodes
 * @param iterations: The number of iterations to run the test
 */
void testMemtableInsertAndSearch(int iterations) {
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];

  clock_t start = clock();

  // Iterative insertion and search
  for (int i = 0; i < iterations; i++) {
    sprintf(key, "key%d", i);
    sprintf(value, "value%d", i);
    insertNodeIntoMemtable(key, value);
    Node *found = searchMemtable(key);
    assert(found != NULL);
    assert(strcmp(found->value, value) == 0);
  }

  printMemoryUsage();
  clock_t end = clock();
  double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC;
  clearMemtable();
  printf("testMemtableInsertAndSearch completed in %.2f seconds.\n", timeTaken);
}

/*
 * void testMemtableRandomInsertAndSearch(int iterations)
 *   Tests the memtable by randomly inserting and searching for nodes
 * @param iterations: The number of iterations to run the test
 */
void testMemtableRandomInsertAndSearch(int iterations) {
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];
  srand((unsigned)time(NULL));

  clock_t start = clock();

  // Random insertion and search
  for (int i = 0; i < iterations; i++) {
    int randKey = rand() % 1000;
    sprintf(key, "key%d", randKey);
    sprintf(value, "value%d", randKey);
    insertNodeIntoMemtable(key, value);
    Node *found = searchMemtable(key);
    assert(found != NULL);
    assert(strcmp(found->value, value) == 0);
  }

  printMemoryUsage();
  clock_t end = clock();
  double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC;
  clearMemtable();
  printf("testMemtableRandomInsertAndSearch completed in %.2f seconds.\n",
         timeTaken);
}

/*
 * void testMemtableRandomDeletion(int iterations)
 *   Tests the memtable by randomly inserting and deleting nodes
 * @param iterations: The number of iterations to run the test
 */
void testMemtableRandomDeletion(int iterations) {
  char key[MAX_KEY_LENGTH];
  int *keys = malloc(iterations * sizeof(int));

  srand(time(NULL));

  clock_t start = clock();

  // Inserting nodes with random keys
  for (int i = 0; i < iterations; i++) {
    keys[i] = rand() % 1000;
    sprintf(key, "key%d", keys[i]);
    insertNodeIntoMemtable(key, "value");
  }

  // Deleting a subset of nodes randomly
  for (int i = 0; i < (iterations / 2); i++) {
    int randIndex = rand() % 100;
    sprintf(key, "key%d", keys[randIndex]);
    deleteMemtableKey(key);

    Node *result = searchMemtable(key);
    assert(result == NULL);
  }

  // print2DMemtable();

  printMemoryUsage();
  clock_t end = clock();
  double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC;

  // clearMemtable();
  printf("testMemtableRandomDeletion completed in %.2f seconds.\n", timeTaken);
}

/*
 * ##########################
 * END Memtree test functions
 * ##########################
 */

/*
 * #############################
 * SSTable system test functions
 * #############################
 */

/*
 * void testSSTableInsertAndSearch(int iterations)
 *   Tests the SSTable by inserting and searching for nodes
 * @param iterations: The number of iterations to run the test
 */
void testSSTableInsertAndSearch(int iterations) {
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];

  clock_t start = clock();

  for (int i = 0; i < iterations; i++) {
    sprintf(key, "key%d", i);
    sprintf(value, "value%d", i);
    write(key, value);
    char *result = read(key);
    assert(strcmp(result, value) == 0);
  }

  printMemoryUsage();
  clock_t end = clock();
  double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC;

  // clearMemtable();
  printf("testSSTableInsertAndSearch completed in %.2f seconds.\n", timeTaken);
}

/*
 * void testSSTableRandomInsert(int iterations)
 *   Tests the SSTable by randomly inserting and searching for nodes
 * @param iterations: The number of iterations to run the test
 */
void testSSTableRandomInsert(int iterations) {
  printf("Starting SSTable read test with %d iterations...\n", iterations);
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];
  srand((unsigned)time(NULL)); // Seed random number generator

  clock_t start = clock();

  // Random key value insertion
  for (int i = 0; i < iterations; i++) {
    int randKey = rand() % iterations;
    sprintf(key, "key%d", randKey);
    sprintf(value, "value%d", randKey);
    write(key, value);
  }

  printMemoryUsage();
  clock_t end = clock();
  double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC;

  // clearMemtable();
  printf("testSSTableRandomInsert completed in %.2f seconds.\n", timeTaken);
}

/*
 * void testSSTableRandomSearch(int iterations)
 *   Tests the SSTable by randomly searching for nodes
 * @param iterations: The number of iterations to run the test
 */
void testSSTableRandomSearch(int iterations) {
  printf("Starting SSTable read test with %d iterations...\n", iterations);
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];
  srand((unsigned)time(NULL)); // Seed random number generator

  clock_t start = clock();

  // Random key search
  for (int i = 0; i < iterations; i++) {
    int randKey = rand() % iterations;
    sprintf(key, "key%d", randKey);
    sprintf(value, "value%d", randKey);
    char *result = read(key);
    assert(strcmp(result, value) == 0);
  }

  printMemoryUsage();
  clock_t end = clock();
  double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC;

  // clearMemtable();
  printf("testSSTableRandomSearch completed in %.2f seconds.\n", timeTaken);
}

/*
 * void testSSTableRandomDeletion(int iterations)
 *   Tests the SSTable by randomly deleting nodes
 * @param iterations: The number of iterations to run the test
 */
void testSSTableRandomDeletion(int iterations) {
  printf("Starting SSTable read test with %d iterations...\n", iterations);
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];
  srand((unsigned)time(NULL)); // Seed random number generator

  clock_t start = clock();

  // Random key deletion
  for (int i = 0; i < iterations; i++) {
    int randKey = rand() % iterations;
    sprintf(key, "key%d", randKey);
    sprintf(value, "value%d", randKey);
    delete (key);
    // char *result = read(key);
    // assert(result == NULL);
  }

  printMemoryUsage();
  clock_t end = clock();
  double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC;

  // clearMemtable();
  printf("testSSTableRandomDeletion completed in %.2f seconds.\n", timeTaken);
}

/*
 * #################################
 * END SSTable system test functions
 * #################################
 */

// Function to run all tests
void runAllTests(int iterations) {
  // testMemtableInsertAndSearch(iterations);
  // testMemtableRandomInsertAndSearch(iterations);
  // testMemtableRandomDeletion(iterations);
  // testSSTableInsertAndSearch(iterations);
  // testSSTableRandomInsert(iterations);
  // testSSTableRandomSearch(iterations);
  // testSSTableRandomDeletion(iterations);
  // print2DMemtable();
  // printf("All tests passed!\n");
}
