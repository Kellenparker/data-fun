#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "memtable.h"
#include "sstable.h"
#include "test.h"

// Function declarations
void testUI();

// Main function; acts as a UI
int main(int argc, char *argv[]) {
  initializeSSTable();
  clearMemtable();
  clearSSTables();
  // runAllTests(100000);

  char command[100];
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];
  char inputBuffer[200];

  while (1) {
    printf("Enter command (write [w], read [r], delete [d], dump [dump], "
           "print memtable [p], test [t], compact [comp]): ");
    fgets(command, sizeof(command), stdin);
    command[strcspn(command, "\n")] = 0; // Remove newline character

    if (strcmp(command, "write") == 0 || strcmp(command, "w") == 0) {
      printf("Enter key and value, separated by a space: ");
      fgets(inputBuffer, sizeof(inputBuffer), stdin);
      sscanf(inputBuffer, "%s %s", key, value);
      write(key, value);
    } else if (strcmp(command, "read") == 0 || strcmp(command, "r") == 0) {
      printf("Enter key: ");
      fgets(key, sizeof(key), stdin);
      key[strcspn(key, "\n")] = 0;
      char *value = read(key);
      if (value != NULL) {
        printf("Value: %s\n", value);
      } else {
        printf("Key not found.\n");
      }
    } else if (strcmp(command, "delete") == 0 || strcmp(command, "d") == 0) {
      printf("Enter key: ");
      fgets(key, sizeof(key), stdin);
      key[strcspn(key, "\n")] = 0;
      delete (key);
    } else if (strcmp(command, "dump") == 0) {
      writeMemtableToSSTable();
      clearMemtable();
    } else if (strcmp(command, "print") == 0 || strcmp(command, "p") == 0) {
      printMemtable();
    } else if (strcmp(command, "clear") == 0 || strcmp(command, "c") == 0) {
      clearSSTables();
      clearMemtable();
    } else if (strcmp(command, "test") == 0 || strcmp(command, "t") == 0) {
      testUI();
    } else if (strcmp(command, "compact") == 0 ||
               strcmp(command, "comp") == 0) {
      compactSSTables();
    } else if (strcmp(command, "q") == 0) {
      break;
    } else {
      printf("Unknown command.\n");
    }
  }

  return 0;
}

/*
 * void testUI()
 *   UI for running tests
 */
void testUI() {
  // void testMemtableInsertAndSearch(int iterations);
  // void testMemtableRandomInsertAndSearch(int iterations);
  // void testMemtableRandomDeletion(int iterations);
  // void testLSMInsertAndSearch(int iterations);
  // void testLSMRandomInsert(int iterations);
  // void testLSMRandomSearch(int iterations);
  // void testLSMRandomDeletion(int iterations);
  printf("Enter test (testMemtableInsertAndSearch [1], "
         "testMemtableRandomInsertAndSearch [2], testMemtableRandomDeletion "
         "[3], testLSMInsertAndSearch [4], testLSMRandomInsert [5], "
         "testLSMRandomSearch [6], testLSMRandomDeletion [7]): ");
  char inputBuffer[200]; // Buffer for fgets
  fgets(inputBuffer, sizeof(inputBuffer), stdin);
  int testNumber;
  sscanf(inputBuffer, "%d", &testNumber);
  printf("Enter number of iterations: ");
  fgets(inputBuffer, sizeof(inputBuffer), stdin);
  int iterations;
  sscanf(inputBuffer, "%d", &iterations);
  switch (testNumber) {
  case 1:
    testMemtableInsertAndSearch(iterations);
    break;
  case 2:
    testMemtableRandomInsertAndSearch(iterations);
    break;
  case 3:
    testMemtableRandomDeletion(iterations);
    break;
  case 4:
    testLSMInsertAndSearch(iterations);
    break;
  case 5:
    testLSMRandomInsert(iterations);
    break;
  case 6:
    testLSMRandomSearch(iterations);
    break;
  case 7:
    testLSMRandomDeletion(iterations);
    break;
  default:
    printf("Unknown test.\n");
    break;
  }
}