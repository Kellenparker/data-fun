#ifndef SSTABLE_H
#define SSTABLE_H

// Directory name that will contain all SSTables and tombstone file
#define DIR_NAME "data"

// SSTable macros
#define FILENAME_FORMAT DIR_NAME "/sstable_%lld.dat"
#define MEMORY_THRESHOLD 1000 * 1024 // 1MB
#define DELIMITER " "                // key[delimiter]value

// Compaction macros and structs
#define TOMBSTONE_FILE "tombstones.dat"
#define TOMBSTONE_PATH DIR_NAME "/" TOMBSTONE_FILE
#define SMALL_FILE_THRESHOLD 200 * 1024  // 200KB
#define UPPER_MERGE_THRESHOLD 400 * 1024 // 400KB
// Struct for tombstone array
typedef struct {
  char **keys;
  int size;
  int capacity;
} TombstoneArray;
// Struct for file path list
typedef struct {
  char **filePaths;
  int size;
  int capacity;
} FilePathList;

// Function declarations
// Writes the memtable to an SSTable
void writeMemtableToSSTable();
// Writes a single entry to the Memtable, writes to SSTable if memory threshold
// is exceeded
void write(char *key, char *value);
// Reads a value from the memtable or SSTable
char *read(char *key);
// Deletes a key from the memtable or SSTable
void delete(char *key);
// Runs compaction process on SSTables
void compactSSTables();
// Clears all SSTables and tombstone file
void clearSSTables();
// Initializes the SSTable system
void initializeSSTable();

#endif // SSTABLE_H