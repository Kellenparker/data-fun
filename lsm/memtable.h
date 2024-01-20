#ifndef MEMTABLE_H
#define MEMTABLE_H

// Define macros for the maximum lengths of keys and values
#define MAX_KEY_LENGTH 100
#define MAX_VALUE_LENGTH 100

// Memory usage macros
// Macro to calculate the memory usage of a node
#define NODE_MEMORY_USAGE(key, value) (sizeof(Node) + strlen(key) + 1 + strlen(value) + 1)
// Macro to increase global memory usage
#define INCREASE_MEMORY_USAGE(key, value) (globalMemoryUsage += NODE_MEMORY_USAGE(key, value))
// Macro to decrease global memory usage
#define DECREASE_MEMORY_USAGE(key, value) (globalMemoryUsage -= NODE_MEMORY_USAGE(key, value))
// Tracks the current memory usage of the memtable
extern int globalMemoryUsage; 


// Node structure for the Binary Search Tree (BST)
typedef struct Node {
  char *key;          // Pointer to the key of the node
  char *value;        // Pointer to the value associated with the key
  struct Node *left;  // Pointer to the left child node
  struct Node *right; // Pointer to the right child node
} Node;

// For purposes of this project, we will use a global memtable
// All external functions will only use this global memtable
extern Node *memtableRoot; 

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
