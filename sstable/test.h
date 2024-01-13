#ifndef TEST_H
#define TEST_H

// Function declarations
void testMemtableInsertAndSearch(int iterations);
void testMemtableRandomInsertAndSearch(int iterations);
void testMemtableRandomDeletion(int iterations);
void testSSTableInsertAndSearch(int iterations);
void testSSTableRandomInsert(int iterations);
void testSSTableRandomSearch(int iterations);
void testSSTableRandomDeletion(int iterations);
void runAllTests(int iterations);

#endif // TEST_H
