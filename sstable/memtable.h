#ifndef MEMTABLE_H
#define MEMTABLE_H

// Define constants for the maximum lengths of keys and values
#define MAX_KEY_LENGTH 100
#define MAX_VALUE_LENGTH 100

// Node structure for the Binary Search Tree (BST)
typedef struct Node {
  char *key;          // Pointer to the key of the node
  char *value;        // Pointer to the value associated with the key
  struct Node *left;  // Pointer to the left child node
  struct Node *right; // Pointer to the right child node
} Node;

// Global variables
// For purposes of this project, we will use a global memtable
// All external functions will only use this global memtable
extern Node *memtableRoot;    // Root node of the BST (memtable)
extern int globalMemoryUsage; // Tracks the total memory usage of the memtable

// Function declarations
// Creates a new BST node with the given key and value
Node *createNode(char *key, char *value);
// Inserts a new key-value pair into the memtable
void insertNodeIntoMemtable(char *key, char *value);
// Searches for a key in the memtable and returns its node
Node *searchMemtable(char *key);
// Deletes a key from the memtable and returns 1 if successful
int deleteMemtableKey(char *key);
// Clears the entire memtable, freeing all nodes
void clearMemtable();
// Performs an inorder traversal of the memtable
void inorderTraversalMemtable();
// Prints an inorder traversal of the memtable and its memory usage
void printMemtable();
// Prints the current memory usage of the memtable
void printMemoryUsage();

#endif // MEMTABLE_H
