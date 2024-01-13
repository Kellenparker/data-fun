#include <sys/stat.h>

// Utility functions

/*
 * static int filenameComparator(const void *a, const void *b)
 *   Function to compare two filenames in descending order
 *   This expects files like: memtable_1705177288571309000.dat
 * @param a: pointer to the first filename
 * @param b: pointer to the second filename
 */
static int filenameComparator(const void *a, const void *b) {
  const char *filenameA = *(const char **)a;
  const char *filenameB = *(const char **)b;
  // printf("Comparing %s and %s\n", filenameA, filenameB);
  return strcmp(filenameB, filenameA);
}

/*
 * static int directoryExists(const char *path)
 *   Function to check if a directory exists
 * @param path: The path to the directory
 * @return: 1 if the directory exists, 0 otherwise
 */
int directoryExists(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0; // Path does not exist or error occurred
    }
    return S_ISDIR(statbuf.st_mode); // Check if it's a directory
}