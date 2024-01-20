#ifndef TEST_H
#define TEST_H

// Function declarations
void testMemtableInsertAndSearch(int iterations);
void testMemtableRandomInsertAndSearch(int iterations);
void testMemtableRandomDeletion(int iterations);
void testLSMInsertAndSearch(int iterations);
void testLSMRandomInsert(int iterations);
void testLSMRandomSearch(int iterations);
void testLSMRandomDeletion(int iterations);
void runAllTests(int iterations);

#endif // TEST_H
