#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "memtable.h"
#include "sstable.h"
#include "util.h"

/*
 * static void sortFilenames(char **filenames, int count)
 *   Sorts an array of filenames in descending order.
 *   Used to find the most recent SSTable file.
 * @param filenames: The array of filenames
 * @param count: The number of filenames in the array
 */
static void sortFilenames(char **filenames, int count) {
  qsort(filenames, count, sizeof(char *), filenameComparator);
}

/*
 * static char *readFromSSTables(char *key)
 *   Attempts to read a key from SSTable files.
 *   First checks a tombstone file for deletion markers, then searches through
 *   sorted SSTable files. Reads the value for a given key and returns it. If a
 *   tombstone or no match is found, returns NULL.
 * @param key: The key to read
 * @return: The value assigned to the key, or NULL if not found
 */
static char *readFromSSTables(char *key) {
  // Check the tombstone file first
  // If the key is found in the tombstone file, return NULL
  FILE *tombstoneFile = fopen(TOMBSTONE_PATH, "r");
  if (tombstoneFile != NULL) {
    char line[256], tombstoneKey[MAX_KEY_LENGTH];
    while (fgets(line, sizeof(line), tombstoneFile)) {
      sscanf(line, "%s", tombstoneKey);
      if (strcmp(key, tombstoneKey) == 0) {
        fclose(tombstoneFile);
        return NULL; // Key has a tombstone, treat as deleted
      }
    }
    fclose(tombstoneFile);
  }

  // Not found in tombstone file, so continue searching in SSTable files
  // Open the data directory
  DIR *dir = opendir(DIR_NAME);
  // dirent (cool):
  // https://pubs.opengroup.org/onlinepubs/009695399/basedefs/dirent.h.html
  // Likely removes support for Windows though
  struct dirent *entry;
  // Array to store filenames
  // 100 is a bit arbitrary, but more than that exceeds the scope of this
  // project
  char *filenames[100];
  // Count of filenames, used to later sort the array
  int count = 0;

  if (dir == NULL) {
    // Directory could not be opened or does not exist
    perror("Failed to open data directory for reading");
    return NULL;
  }

  while ((entry = readdir(dir)) != NULL) {
    // Add the filename to the array if it is a regular data file
    if (entry->d_type == DT_REG && strcmp(entry->d_name, TOMBSTONE_FILE) != 0) {
      filenames[count++] = strdup(entry->d_name); // Store filename
    }
  }
  // We are done with the directory at this point
  closedir(dir);

  // Sort filenames in descending order
  sortFilenames(filenames, count);

  // Variables for reading from SSTable files
  char filepath[256], line[256], fileKey[MAX_KEY_LENGTH],
      fileValue[MAX_VALUE_LENGTH];
  char *foundValue = NULL;

  // Iterate through sorted SSTable files
  for (int i = 0; i < count; i++) {
    snprintf(filepath, sizeof(filepath), "%s/%s", DIR_NAME, filenames[i]);
    printf("Reading from SSTable file: %s\n", filepath);
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
      perror("Failed to open SSTable file for reading");
      // TODO: Since this is unexpected, maybe we should just break?
      continue;
    }

    // Read each line of the file
    while (fgets(line, sizeof(line), file)) {
      sscanf(line, "%s %s", fileKey, fileValue);
      // Check if the key matches
      if (strcmp(key, fileKey) == 0) {
        foundValue = strdup(fileValue);
        break;
      }
    }
    fclose(file);
    if (foundValue != NULL) {
      break;
    }
  }

  // Free allocated filenames
  for (int i = 0; i < count; i++) {
    free(filenames[i]);
  }

  return foundValue; // Returns NULL if key is not found
}

/*
 * char *read(char *key)
 *   Public function to read a key from the memtable or SSTable files.
 * @param key: The key to read
 * @return: The value assigned to the key, or NULL if not found
 */
char *read(char *key) {
  // First, check the memtable
  Node *node = searchMemtable(key);
  if (node != NULL) {
    // Key found in memtable, nice!
    return node->value;
  } else {
    // Key not found in memtable, now we check SSTable files
    char *value = readFromSSTables(key);
    // If value is NULL, it was either deleted or not found
    if (value != NULL) {
      return value;
    } else {
      return NULL;
    }
  }
}

/*
 * static void initializeDataDirectory()
 *   Creates the data directory if it does not exist
 */
static void initializeDataDirectory() {
  const char *dirName = "data";
  struct stat st = {0};

  if (stat(dirName, &st) == -1) {
    // Data directory does not exist, create it with read/write permissions
    mkdir(dirName, 0700);
  }
}

/*
 * static char *generateUniqueFilename()
 *   Generates a unique filename for an SSTable file.
 *   The filename is based on the current time in nanoseconds.
 * @return: The dynamically allocated filename
 */
static char *generateUniqueFilename() {
  char *filename = malloc(256 * sizeof(char));
  if (filename == NULL) {
    perror("Failed to allocate memory for filename");
    return NULL;
  }

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  // Normal time() function is not precise enough, I imagine nanoseconds are...
  // https://ftp.gnu.org/old-gnu/Manuals/glibc-2.2.3/html_node/libc_418.html
  long long now = ts.tv_sec * 1000000000LL + ts.tv_nsec;

  // Format the filename
  snprintf(filename, 256, FILENAME_FORMAT, now);

  return filename;
}

/*
 * static void serializeMemtableToFile(Node *root, FILE *file)
 *    Recursively prints the memtable to a file in-order.
 *    Could live in memtable.c??
 * @param root: The root of the memtable
 * @param file: The file to write to
 */
static void serializeMemtableToFile(Node *root, FILE *file) {
  if (root == NULL) {
    return;
  }

  serializeMemtableToFile(root->left, file);
  fprintf(file, "%s %s\n", root->key, root->value);
  serializeMemtableToFile(root->right, file);
}

/*
 * static void writeMemtableToSSTable()
 *   Writes the memtable to an SSTable file.
 */
void writeMemtableToSSTable() {
  // Check if the data directory exists
  if (!directoryExists(DIR_NAME)) {
    // We could initialize here, but it not existing is not expected
    perror("Data directory does not exist, could not write SSTable file");
    return;
  }

  // Generate a timestamped filename
  char *filename = generateUniqueFilename();

  if (filename == NULL) {
    perror("Failed to generate unique filename");
    return;
  }

  // Open the new file for writing
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    perror("Failed to open SSTable file for writing");
    return;
  }

  // Write the memtable to the file
  serializeMemtableToFile(memtableRoot, file);

  free(filename);
  fclose(file);
  printf("Memtable written to SSTable file: %s\n", filename);
}

/*
 * void write(char *key, char *value)
 *   Public function to write a key-value pair to the system.
 *   Will first write to the memtable, then check if the memory usage is above
 *   the threshold. If so, the memtable will be written to an SSTable file
 * @param key: The key to be written
 * @param value: The value to be written
 */
void write(char *key, char *value) {
  // Check if key or value is null
  if (key == NULL || value == NULL) {
    printf("Key or value cannot be null.\n");
    return;
  }
  // Check if key or value exceeds the maximum length
  if (strlen(key) > MAX_KEY_LENGTH || strlen(value) > MAX_VALUE_LENGTH) {
    printf("Key or value exceeds maximum length of %d.\n", (int)MAX_KEY_LENGTH);
    return;
  }
  insertNodeIntoMemtable(key, value);
  // Check if the memory usage is above the memtable threshold
  if (globalMemoryUsage > MEMORY_THRESHOLD) {
    // Write the memtable to an SSTable file and clear the memtable
    writeMemtableToSSTable();
    clearMemtable();
  }
}

/*
 * static void initializeTombstoneFile()
 *   Creates the tombstone file if it does not exist
 */
static void initializeTombstoneFile() {
  FILE *file = fopen(TOMBSTONE_PATH, "w");
  if (file == NULL) {
    perror("Failed to open tombstone file for writing");
    return;
  }
  fclose(file);
}

/*
 * static void writeTombstone(char *key)
 *   Writes a single tombstone to the tombstone file.
 * @param key: The key to be marked as deleted
 */
static void writeTombstone(char *key) {
  FILE *file = fopen(TOMBSTONE_PATH, "a"); // Open for appending

  if (file == NULL) {
    perror("Failed to open tombstone file for writing");
    return;
  }

  fprintf(file, "%s\n", key);
  fclose(file);
}

/*
 * void delete(char *key)
 *   Public function to delete a key from the system.
 *   Will first attempt to delete from the memtable, and if not found in the
 *   memtable, create a tombstone for the key.
 * @param key: The key to be deleted
 */
void delete(char *key) {
  // Try to delete from the memtable
  if (!deleteMemtableKey(key)) {
    // Key was not in the memtable, create a tombstone
    writeTombstone(key);
  } else {
    printf("Key deleted from memtable: %s\n", key);
  }
}

/*
 * static void initializeTombstoneArray(TombstoneArray *array)
 *   Allocates memory for the tombstone array
 * @param array: Pointer to the tombstone array
 */
void initializeTombstoneArray(TombstoneArray *array) {
  array->size = 0;
  array->capacity = 10; // Initial capacity
  array->keys = malloc(array->capacity * sizeof(char *));
  if (array->keys == NULL) {
    perror("Failed to allocate memory for tombstone array");
    exit(EXIT_FAILURE);
  }
}

/*
 * static void addTombstone(TombstoneArray *array, const char *key)
 *   Adds a tombstone to the tombstone array.
 *   If the array is full, double it in true C fashion.
 * @param array: Pointer to the tombstone array
 * @param key: The key to be added to the array
 */
void addTombstone(TombstoneArray *array, const char *key) {
  // Check if the array is full
  if (array->size >= array->capacity) {
    // If so, double it
    array->capacity *= 2;
    char **temp = realloc(array->keys, array->capacity * sizeof(char *));
    if (temp == NULL) {
      perror("Failed to reallocate memory for tombstone array");
      exit(EXIT_FAILURE);
    }
    array->keys = temp;
  }
  // Add the key to the array
  array->keys[array->size++] = strdup(key);
}

/*
 * static void loadTombstones(TombstoneArray *array, const char
 * *tombstoneFilename) Loads tombstones from the tombstone file into the
 * tombstone array. Also clears the tombstone file.
 * @param tombstones: Pointer to the tombstone array
 * @param tombstoneFilename: The filename of the tombstone file
 */
void loadTombstones(TombstoneArray *tombstones, const char *tombstoneFilename) {
  FILE *file = fopen(tombstoneFilename, "r");
  char line[256];

  if (file == NULL) {
    perror("Failed to open tombstone file");
    return;
  }

  // Read the tombstone file
  while (fgets(line, sizeof(line), file)) {
    char *newline = strchr(line, '\n');
    if (newline) {
      *newline = '\0';
    }
    // Add the key to the tombstone array
    addTombstone(tombstones, line);
  }

  // Clear the tombstone file
  freopen(tombstoneFilename, "w", file);

  fclose(file);
}

/*
 * static int containsTombstone(...)
 *   Checks the tombstone array to see if the given key exists.
 *   Note: Linear search is gross, replacing the array with a hash table would
 *   be ideal, but time constraints...
 * @param tombstones: Pointer to the tombstone array
 * @param key: The key to be checked
 */
int containsTombstone(const TombstoneArray *tombstones, const char *key) {
  for (int i = 0; i < tombstones->size; i++) {
    if (strcmp(tombstones->keys[i], key) == 0) {
      // Key found in tombstone array
      return 1;
    }
  }
  // Key not found in tombstone array
  return 0;
}

/*
 * static void applyTombstonesToFile(...)
 *   Applies tombstones to an SSTable file.
 *   Reads the SSTable file line by line, and if the key is found in the
 *   tombstone array, the line is not written to the temporary file.
 * @param filepath: The filepath of the SSTable file
 * @param tombstones: Pointer to the tombstone array
 */
void applyTombstonesToFile(const char *filepath,
                           const TombstoneArray *tombstones) {
  // Open the SSTable file for reading
  FILE *file = fopen(filepath, "r");
  if (file == NULL) {
    perror("Failed to open SSTable file for reading");
    return;
  }

  // Create a temporarly named file to write updated entries
  char tempFilepath[256];
  snprintf(tempFilepath, sizeof(tempFilepath), "%s.temp", filepath);
  FILE *tempFile = fopen(tempFilepath, "w");
  if (tempFile == NULL) {
    perror("Failed to open temporary file for writing");
    fclose(file);
    return;
  }

  // Variables for reading from the SSTable file
  char line[256], key[MAX_KEY_LENGTH], value[MAX_VALUE_LENGTH];

  // Process each entry in the SSTable file
  while (fgets(line, sizeof(line), file)) {
    sscanf(line, "%s %s", key, value);
    if (!containsTombstone(tombstones, key)) {
      // Key not found in tombstone array, so write the entry
      fputs(line, tempFile);
    } else {
      // printf("Key deleted from SSTable via tombstone: %s\n", key);
    }
  }

  fclose(file);
  fclose(tempFile);

  // Replace the temporary name with the original name
  remove(filepath);
  rename(tempFilepath, filepath);
}

/*
 * static void freeTombstoneArray(TombstoneArray *array)
 *   Frees the tombstone array when it is no longer needed.
 * @param array: Pointer to the tombstone array
 */
void freeTombstoneArray(TombstoneArray *array) {
  for (int i = 0; i < array->size; i++) {
    free(array->keys[i]);
  }
  free(array->keys);
  array->keys = NULL;
  array->size = 0;
  array->capacity = 0;
}

/*
 * static void initializeList(FilePathList *list)
 *   Allocates memory for the list of filepaths.
 * @param list: Pointer to the list of filepaths
 */
static void initializeList(FilePathList *list) {
  list->size = 0;
  // Initial capacity of 10
  list->capacity = 10;
  list->filePaths = malloc(list->capacity * sizeof(char *));
  if (list->filePaths == NULL) {
    perror("Failed to allocate memory for list");
    exit(EXIT_FAILURE);
  }
}

/*
 * static void addToList(...)
 *   Adds a filepath to the list of filepaths.
 *   If the list is full, double it
 * @param list: Pointer to the list of filepaths
 * @param filepath: The filepath to be added to the list
 */
static void addToList(FilePathList *list, const char *filepath) {
  if (list->size >= list->capacity) {
    // Resize the array when capacity is reached
    list->capacity *= 2;
    char **temp = realloc(list->filePaths, list->capacity * sizeof(char *));
    if (temp == NULL) {
      perror("Failed to reallocate memory for list");
      exit(EXIT_FAILURE);
    }
    list->filePaths = temp;
  }
  // Add the filepath to the list
  list->filePaths[list->size] = strdup(filepath);
  if (list->filePaths[list->size] == NULL) {
    perror("Failed to duplicate filepath");
    exit(EXIT_FAILURE);
  }
  list->size++;
}

/*
 * static void deleteMergedFile(const char *filePath)
 *   Deletes a merged file.
 * @param filePath: The filepath of the merged file
 */
static void deleteMergedFile(const char *filePath) {
  if (remove(filePath) != 0) {
    perror("Failed to delete the original small SSTable file");
  }
}

/*
 * static void mergeSmallFiles(FilePathList *list)
 *   Merges small SSTable files into larger SSTable files.
 *   Will merge as many files as possible into a single file, as long as the
 *   upper threshold is not exceeded.
 *   TODO: Check if same key exists in merging files and keep the most recent
 * @param list: Pointer to the list of filepaths of small SSTable files
 *    (files that are below the lower threshold)
 */
static void mergeSmallFiles(FilePathList *list) {
  if (list->size < 2) {
    // Not enough files to merge
    return;
  }

  // Variables for merging files
  long mergedFileSize = 0;
  int filesMerged = 0;
  char *filename = generateUniqueFilename();
  FILE *mergedFile = fopen(filename, "w");

  // Iterate through the list of filepaths
  for (int i = 0; i < list->size; i++) {
    // Get the size of the file
    struct stat st;
    if (stat(list->filePaths[i], &st) != 0) {
      // Cannot get file info, skip this file
      // TODO: error?
      continue;
    }

    // Check if the upper threshold will be exceeded
    if (mergedFileSize + st.st_size > UPPER_MERGE_THRESHOLD) {
      // Close current merged file and start a new one, but only if at least
      // one file was merged
      if (filesMerged > 0) {
        fclose(mergedFile);
        filename = generateUniqueFilename();
        mergedFile = fopen(filename, "w");
        mergedFileSize = 0;
        filesMerged = 0;
      }
    }

    // Open the small SSTable file for reading
    FILE *smallFile = fopen(list->filePaths[i], "r");
    if (!smallFile) {
      perror("Failed to open small SSTable file for merging");
      continue;
    }

    // Write each line of the small SSTable file to the merged file
    char line[256];
    while (fgets(line, sizeof(line), smallFile)) {
      fputs(line, mergedFile);
      mergedFileSize += strlen(line);
    }
    fclose(smallFile);

    // Call the function to delete the merged file
    deleteMergedFile(list->filePaths[i]);

    filesMerged++;
  }

  // Close the last merged file if any merging was done
  if (filesMerged > 0) {
    fclose(mergedFile);
  }
}

/*
 * static void clearList(FilePathList *list)
 *   Frees the list of filepaths when it is no longer needed.
 * @param list: Pointer to the list of filepaths
 */
static void clearList(FilePathList *list) {
  for (int i = 0; i < list->size; i++) {
    free(list->filePaths[i]);
  }
  free(list->filePaths);
  list->filePaths = NULL;
  list->size = 0;
  list->capacity = 0;
}

/*
 * static void removeOldFiles(FilePathList *list)
 *   Removes the old SSTable files that were merged.
 * @param list: Pointer to the list of filepaths
 */
static void removeOldFiles(FilePathList *list) {
  for (int i = 0; i < list->size; i++) {
    if (remove(list->filePaths[i]) != 0) {
      perror("Error removing file");
    } else {
      printf("Removed file: %s\n", list->filePaths[i]);
    }
  }
}

/*
 * static int isFileBelowThreshold(...)
 *   Checks if a file is below a set threshold size.
 * @param filepath: The filepath of the file to check
 */
static int isFileBelowThreshold(const char *filepath) {
  struct stat st;
  if (stat(filepath, &st) != 0) {
    perror("Failed to get file info");
    return 0;
  }
  return st.st_size < SMALL_FILE_THRESHOLD;
}

/*
 * void compactSSTables()
 *   Public function to compact SSTable files.
 *   Two main purposes:
 *     1. Identify small SSTable files and merge them into larger files
 *     2. Apply tombstones to SSTable files
 *   Improvements:
 *     1. Delete duplicate keys that exist in multiple SSTable files
 */
void compactSSTables() {
  DIR *dir = opendir(DIR_NAME);
  struct dirent *entry;
  char filepath[256];

  if (dir == NULL) {
    perror("Failed to open data directory for compaction");
    return;
  }

  // List to store file paths of small SSTables for merging
  FilePathList smallFilesList;
  initializeList(&smallFilesList);

  // Initialize and load tombstones
  TombstoneArray tombstones;
  initializeTombstoneArray(&tombstones);
  loadTombstones(&tombstones, TOMBSTONE_PATH);

  // Step 1: Identify small SSTable files
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      if (strcmp(entry->d_name, TOMBSTONE_FILE) == 0) {
        printf("Skipping %s\n", entry->d_name);
        continue; // Skip tombstone file
      }
      snprintf(filepath, sizeof(filepath), "%s/%s", DIR_NAME, entry->d_name);
      // Apply tombstones to the SSTable file
      applyTombstonesToFile(filepath, &tombstones);
      // If the file is below the threshold after applying tombstones, add it to
      // smallFilesList
      if (isFileBelowThreshold(filepath)) {
        addToList(&smallFilesList, filepath);
      }
    }
  }
  closedir(dir);

  // Step 2: Merge small SSTable files
  mergeSmallFiles(&smallFilesList);

  // Clean up
  removeOldFiles(&smallFilesList);
  clearList(&smallFilesList);
}

/*
 * void clearSSTables()
 *   Public function to clear all SSTable files.
 */
void clearSSTables() {
  DIR *dir = opendir(DIR_NAME);
  struct dirent *entry;
  char filepath[256];

  if (dir == NULL) {
    return;
  }

  while ((entry = readdir(dir)) != NULL) {
    // Remove all files
    if (entry->d_type == DT_REG) {
      snprintf(filepath, sizeof(filepath), "%s/%s", DIR_NAME, entry->d_name);
      remove(filepath);
    }
  }

  closedir(dir);
}

/*
 * void initializeSSTable()
 *   Public function to initialize the SSTable system.
 *   Creates the data directory if it does not exist, and creates the tombstone
 *   file if it does not exist.
 */
void initializeSSTable() {
  initializeDataDirectory();
  initializeTombstoneFile();
}