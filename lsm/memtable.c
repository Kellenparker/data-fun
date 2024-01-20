#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memtable.h"

// Global variables initialization
Node *memtableRoot = NULL;
int globalMemoryUsage = 0;

/*
 * Node *createNode(char *key, char *value)
 *   Creates a new node with the given key and value
 * @param key: The key of the new node
 * @param value: The value of the new node
 * @return: A pointer to the new node
 */
Node *createNode(char *key, char *value) {
  // Allocate memory for the new node
  Node *newNode = (Node *)malloc(sizeof(Node));
  // If allocation fails, print error message and return NULL
  if (!newNode) {
    perror("Failed to allocate memory for new node");
    return NULL;
  }

  // Allocate memory for the key and value of the new node
  newNode->key = strdup(key);
  newNode->value = strdup(value);

  // If allocation fails, print error message and free memory
  if (!newNode->key || !newNode->value) {
    perror("Failed to allocate memory for key or value");
    free(newNode->key);
    free(newNode->value);
    free(newNode);
    return NULL;
  }

  // Set the left and right child nodes to NULL
  newNode->left = newNode->right = NULL;

  // Update the global memory usage
  // We have to manually add the size of the key and value
  INCREASE_MEMORY_USAGE(key, value);

  return newNode;
}

/*
 * static void insertHelper(Node **node, char *key, char *value)
 *   A recursive helper function that finds the correct position to insert a new
 *   node into the BST.
 * @param node: A double pointer to the current node (or root for the first
 *   call).
 * @param key: The key of the new node.
 * @param value: The value of the new node.
 */
static void insertHelper(Node **node, char *key, char *value) {
  if (*node == NULL) {
    *node = createNode(key, value);
  } else if (strcmp(key, (*node)->key) < 0) {
    insertHelper(&((*node)->left), key, value);
  } else if (strcmp(key, (*node)->key) > 0) {
    insertHelper(&((*node)->right), key, value);
  }
}

/*
 * void insertNodeIntoMemtable(char *key, char *value)
 *   Public function to insert a new key-value pair into the memtable.
 * @param key: The key to be inserted into the memtable.
 * @param value: The value associated with the key.
 */
void insertNodeIntoMemtable(char *key, char *value) {
  // Check if key or value exceeds the maximum length
  // TODO: Handle key and value separately?
  if (strlen(key) > MAX_KEY_LENGTH || strlen(value) > MAX_VALUE_LENGTH) {
    printf("Key or value exceeds maximum length of %d.\n", (int)MAX_KEY_LENGTH);
    return;
  }
  insertHelper(&memtableRoot, key, value);
}

/*
 * static Node *search(Node *root, char *key)
 *   Recursively searches for a key in the BST.
 * @param root: The root node (or current node for recursive calls)
 * @param key: The key to be searched for.
 */
static Node *search(Node *root, char *key) {
  // Base cases: root is null or key is present at root
  if (root == NULL || strcmp(root->key, key) == 0) {
    return root;
  }

  // Value is greater than root's key
  if (strcmp(root->key, key) < 0) {
    return search(root->right, key);
  }

  // Value is smaller than root's key
  return search(root->left, key);
}

/*
 * Node *searchMemtable(char *key)
 *   Public function to search for a key in the memtable.
 * @param key: The key to be searched for.
 * @return: A pointer to the node containing the key, or NULL if not found.
 */
Node *searchMemtable(char *key) { return search(memtableRoot, key); }

/*
 * static Node *minValueNode(Node *node)
 *   A utility function to find the node with the minimum key value in the given
 *   subtree. It traverses the tree to the leftmost node which is the node with
 *   the minimum key.
 * @param node: A pointer to the root node of the subtree.
 * @return: A pointer to the node with the minimum key value in the given
 *   subtree.
 */
static Node *minValueNode(Node *node) {
  Node *current = node;

  // Find the leftmost leaf
  while (current && current->left != NULL)
    current = current->left;

  return current;
}

/*
 * static int deleteNodeHelper(Node **node, char *key)
 *   A recursive helper function to delete a node with a given key from the BST.
 *   It finds the node and performs deletion according to BST rules.
 * @param node: A double pointer to the root node of the BST.
 * @param key: The key of the node to be deleted.
 * @return: 1 if deletion is successful, 0 if the key is not found in the tree.
 */
static int deleteNodeHelper(Node **node, char *key) {
  if (*node == NULL) {
    return 0; // Node not found, return 0
  }

  // Recur(is that a word?) down the tree
  if (strcmp(key, (*node)->key) < 0) {
    return deleteNodeHelper(&((*node)->left), key);
  } else if (strcmp(key, (*node)->key) > 0) {
    return deleteNodeHelper(&((*node)->right), key);
  } else {
    // Node with the key found; perform deletion

    Node *temp = *node; // Temporary pointer to the node to be deleted

    // Node with only one child or no child
    if ((*node)->left == NULL || (*node)->right == NULL) {
      *node = ((*node)->left) ? (*node)->left : (*node)->right;
    } else {
      // Node with two children: Get the inorder successor (smallest in the
      // right subtree)
      Node *minNode = minValueNode((*node)->right);

      // Free the old key/value and replace with inorder successor's key/value
      free((*node)->key);
      free((*node)->value);
      (*node)->key = strdup(minNode->key);
      (*node)->value = strdup(minNode->value);

      // Delete the inorder successor
      deleteNodeHelper(&((*node)->right), minNode->key);
    }

    // If the deleted node is different from the temporary node, update memory
    // usage
    if (temp != *node) {
      DECREASE_MEMORY_USAGE(temp->key, temp->value);
      // Free the memory of the temporary node
      free(temp->key);
      free(temp->value);
      free(temp);
    }
    return 1; // Deletion successful
  }
}

/*
 * int deleteMemtableKey(char *key)
 *   Public function to delete a key from the memtable.
 *   It uses the deleteNodeHelper function to perform the deletion on the
 *   memtable.
 * @param key: The key to be deleted.
 * @return: 1 if the deletion is successful, 0 if the key is not found in the
 *   memtable. (needed for SSTable deletion)
 */
int deleteMemtableKey(char *key) {
  return deleteNodeHelper(&memtableRoot, key);
}

/*
 * static Node *clearTree(Node *root)
 *   Recursively clears the entire tree, freeing all the memory used by
 *   nodes. It traverses the tree post-order and frees each node along with
 *   its key and value.
 * @param root: A pointer to the root node of the tree.
 * @return: NULL (hopefully), as the tree is cleared and there are no nodes
 * left.
 */
static Node *clearTree(Node *root) {
  if (root != NULL) {
    clearTree(root->left);  // Clear left subtree
    clearTree(root->right); // Clear right subtree

    // Update global memory usage
    // Note: we could set this to 0, but I like to do it manually
    // so we can detect memory leaks
    DECREASE_MEMORY_USAGE(root->key, root->value);

    // Free the memory!
    free(root->key);
    free(root->value);
    free(root);
  }
  return NULL;
}

/*
 * void clearMemtable()
 *   Clears the entire memtable using clearTree.
 */
void clearMemtable() {
  memtableRoot = clearTree(memtableRoot); // Clear the entire memtable

  // Check for memory leaks
  if (globalMemoryUsage != 0) {
    printf("Memory leak detected. Memory usage: %d\n", globalMemoryUsage);
  }

  // Reset global memory usage
  globalMemoryUsage = 0;
}

/*
 * void inorderTraversal(Node *root)
 *   Recursively performs an in-order traversal of the BST.
 *   Prints the key and value of each node.
 * @param root: A pointer to the root node (or current node for recursive calls)
 */
void inorderTraversal(Node *root) {
  if (root != NULL) {
    inorderTraversal(root->left);
    printf("%s, %s \n", root->key, root->value);
    inorderTraversal(root->right);
  }
}

/*
 * void inorderTraversalMemtable()
 *   Performs an in-order traversal of the memtable using the inorderTraversal
 *   function
 */
void inorderTraversalMemtable() { inorderTraversal(memtableRoot); }

/*
 * void printMemtable()
 *   Prints an in-order traversal of the memtable and its memory usage.
 */
void printMemtable() {
  inorderTraversalMemtable();
  printMemoryUsage();
}

/*
 * void printMemoryUsage()
 *   Prints the current memory usage of the memtable.
 */
void printMemoryUsage() {
  printf("Memory usage: %d bytes\n", globalMemoryUsage);
}