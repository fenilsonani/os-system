#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define MAX_FILES 100
#define MAX_FILENAME 50
#define MAX_FILESIZE 256
#define HASH_TABLE_SIZE 127  // Prime number for better distribution

typedef enum {
    FS_SUCCESS = 0,
    FS_ERROR_NULL_POINTER = -1,
    FS_ERROR_FILE_EXISTS = -2,
    FS_ERROR_FILE_NOT_FOUND = -3,
    FS_ERROR_NO_SPACE = -4,
    FS_ERROR_INVALID_NAME = -5,
    FS_ERROR_INIT_FAILED = -6
} FileSystemError;

typedef struct File {
    char filename[MAX_FILENAME];
    char data[MAX_FILESIZE];
    int size;
    int is_deleted;
    struct timespec created_time;
    struct timespec modified_time;
    int access_count;
    struct File* hash_next;  // For hash table chaining
} File;

typedef struct {
    File* hash_table[HASH_TABLE_SIZE];
    File file_pool[MAX_FILES];
    pthread_rwlock_t lock;
    int file_count;
    int total_files_created;
    int total_files_deleted;
    double total_lookup_time_ms;
    int total_lookups;
} FileSystem;

static FileSystem fs;

static unsigned int hash_function(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % HASH_TABLE_SIZE;
}

static double timespec_to_ms(struct timespec *ts) {
    return ts->tv_sec * 1000.0 + ts->tv_nsec / 1000000.0;
}

FileSystemError init_filesystem() {
    memset(&fs, 0, sizeof(FileSystem));
    
    // Initialize hash table
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        fs.hash_table[i] = NULL;
    }
    
    // Initialize file pool
    for (int i = 0; i < MAX_FILES; i++) {
        fs.file_pool[i].is_deleted = 1;
        fs.file_pool[i].hash_next = NULL;
    }
    
    if (pthread_rwlock_init(&fs.lock, NULL) != 0) {
        return FS_ERROR_INIT_FAILED;
    }
    
    printf("Enhanced File System initialized:\n");
    printf("  - Hash table with %d buckets for O(1) lookups\n", HASH_TABLE_SIZE);
    printf("  - Thread-safe read-write operations\n");
    printf("  - Support for %d files\n\n", MAX_FILES);
    
    return FS_SUCCESS;
}

static File* find_file_in_hash(const char* name) {
    unsigned int hash = hash_function(name);
    File* current = fs.hash_table[hash];
    
    while (current != NULL) {
        if (!current->is_deleted && strcmp(current->filename, name) == 0) {
            return current;
        }
        current = current->hash_next;
    }
    return NULL;
}

static File* get_free_file_slot() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.file_pool[i].is_deleted) {
            return &fs.file_pool[i];
        }
    }
    return NULL;
}

FileSystemError create_file(const char* name, const char* data) {
    if (!name || !data) return FS_ERROR_NULL_POINTER;
    if (strlen(name) == 0 || strlen(name) >= MAX_FILENAME) return FS_ERROR_INVALID_NAME;
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    pthread_rwlock_wrlock(&fs.lock);
    
    // Check if file already exists using hash table
    if (find_file_in_hash(name) != NULL) {
        pthread_rwlock_unlock(&fs.lock);
        printf("Error: File already exists: %s\n", name);
        return FS_ERROR_FILE_EXISTS;
    }
    
    // Get a free slot
    File* file = get_free_file_slot();
    if (!file) {
        pthread_rwlock_unlock(&fs.lock);
        printf("Error: File system is full. Cannot create file %s\n", name);
        return FS_ERROR_NO_SPACE;
    }
    
    // Populate the file
    strncpy(file->filename, name, MAX_FILENAME - 1);
    file->filename[MAX_FILENAME - 1] = '\0';
    
    size_t data_len = strlen(data);
    if (data_len >= MAX_FILESIZE) {
        printf("Warning: Data for file '%s' truncated to %d bytes.\n", name, MAX_FILESIZE - 1);
    }
    
    strncpy(file->data, data, MAX_FILESIZE - 1);
    file->data[MAX_FILESIZE - 1] = '\0';
    file->size = strlen(file->data);
    file->is_deleted = 0;
    file->access_count = 0;
    
    clock_gettime(CLOCK_MONOTONIC, &file->created_time);
    file->modified_time = file->created_time;
    
    // Add to hash table
    unsigned int hash = hash_function(name);
    file->hash_next = fs.hash_table[hash];
    fs.hash_table[hash] = file;
    
    fs.file_count++;
    fs.total_files_created++;
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    struct timespec diff;
    if (end_time.tv_nsec < start_time.tv_nsec) {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec - 1;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
    }
    
    double create_time = timespec_to_ms(&diff);
    
    pthread_rwlock_unlock(&fs.lock);
    
    printf("Created File: %s (Size: %d bytes, %.3f ms)\n", 
           file->filename, file->size, create_time);
    return FS_SUCCESS;
}

FileSystemError read_file(const char* name) {
    if (!name) return FS_ERROR_NULL_POINTER;
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    pthread_rwlock_rdlock(&fs.lock);
    
    File* file = find_file_in_hash(name);
    if (!file) {
        pthread_rwlock_unlock(&fs.lock);
        printf("File not found: %s\n", name);
        return FS_ERROR_FILE_NOT_FOUND;
    }
    
    file->access_count++;
    
    printf("Reading File: %s\n", name);
    printf("  Size: %d bytes\n", file->size);
    printf("  Access count: %d\n", file->access_count);
    printf("  Content: %s\n", file->data);
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    struct timespec diff;
    if (end_time.tv_nsec < start_time.tv_nsec) {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec - 1;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
    }
    
    double lookup_time = timespec_to_ms(&diff);
    fs.total_lookup_time_ms += lookup_time;
    fs.total_lookups++;
    
    pthread_rwlock_unlock(&fs.lock);
    
    printf("  Lookup time: %.3f ms\n", lookup_time);
    return FS_SUCCESS;
}

int delete_file(const char* name) {
    for (int i = 0; i < file_count; i++) {
        if (!file_system[i].is_deleted && strcmp(file_system[i].filename, name) == 0) {
            file_system[i].is_deleted = 1;
            // Clear data to simulate deletion more accurately (optional)
            memset(file_system[i].data, 0, MAX_FILESIZE);
            file_system[i].size = 0;
            printf("Deleted File: %s (Slot: %d)\n", name, i);
            return 0; // Success
        }
    }
    printf("Error: File not found for deletion: %s\n", name);
    return -1; // Failure
}

void list_files() {
    printf("File Listing:\n");
    printf("----------------------------------------\n");
    printf("%-20s %-10s\n", "Filename", "Size (bytes)");
    printf("----------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < file_count; i++) {
        if (!file_system[i].is_deleted) {
            printf("%-20s %-10d\n", file_system[i].filename, file_system[i].size);
            count++;
        }
    }
    
    if (count == 0) {
        printf("No files found.\n");
    }
    printf("----------------------------------------\n");
    printf("Total files: %d\n", count);
}

int main() {
    printf("Simple File System Demo\n");
    printf("======================\n\n");
    
    create_file("test1.txt", "Hello, World!");
    create_file("test2.txt", "Operating System Concepts.");
    create_file("notes.txt", "This is a note about file systems implementation.");
    
    list_files();
    
    printf("\n--- Deleting test1.txt ---\n");
    delete_file("test1.txt");
    list_files();

    printf("\n--- Creating large_file.txt (will reuse slot) ---\n");
    // This should reuse slot 0 where test1.txt was
    create_file("large_file.txt", "This is a larger file that should reuse a previously deleted slot.");
    list_files();

    printf("\n--- Reading Files ---\n");
    read_file("test2.txt");
    read_file("test1.txt"); // Should fail (deleted)
    read_file("large_file.txt"); // Should succeed
    read_file("non_existent.txt"); // Should fail

    printf("\n--- Attempting to create existing file ---\n");
    create_file("test2.txt", "Duplicate content"); // Should fail

    printf("\nFile system demo finished.\n");
    return 0;
} 