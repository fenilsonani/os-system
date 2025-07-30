#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FILES 100
#define MAX_FILENAME 50
#define MAX_FILESIZE 256

typedef struct {
    char filename[MAX_FILENAME];
    char data[MAX_FILESIZE];
    int size;
    int is_deleted;
} File;

File file_system[MAX_FILES];
int file_count = 0;

// O(n) linear search for file creation
int create_file(const char* name, const char* data) {
    // Check if file already exists - O(n) search
    for (int i = 0; i < file_count; i++) {
        if (!file_system[i].is_deleted && strcmp(file_system[i].filename, name) == 0) {
            printf("Error: File already exists: %s\n", name);
            return -1;
        }
    }
    
    if (file_count >= MAX_FILES) {
        printf("Error: File system is full\n");
        return -1;
    }
    
    strncpy(file_system[file_count].filename, name, MAX_FILENAME - 1);
    file_system[file_count].filename[MAX_FILENAME - 1] = '\0';
    strncpy(file_system[file_count].data, data, MAX_FILESIZE - 1);
    file_system[file_count].data[MAX_FILESIZE - 1] = '\0';
    file_system[file_count].size = strlen(file_system[file_count].data);
    file_system[file_count].is_deleted = 0;
    
    printf("Created File: %s (Size: %d bytes)\n", name, file_system[file_count].size);
    file_count++;
    return file_count - 1;
}

// O(n) linear search for file reading
int read_file(const char* name) {
    for (int i = 0; i < file_count; i++) {
        if (!file_system[i].is_deleted && strcmp(file_system[i].filename, name) == 0) {
            printf("Reading File: %s (Size: %d bytes)\n", name, file_system[i].size);
            printf("Content: %s\n", file_system[i].data);
            return 0;
        }
    }
    printf("File not found: %s\n", name);
    return -1;
}

// O(n) linear search for file deletion
int delete_file(const char* name) {
    for (int i = 0; i < file_count; i++) {
        if (!file_system[i].is_deleted && strcmp(file_system[i].filename, name) == 0) {
            file_system[i].is_deleted = 1;
            printf("Deleted File: %s\n", name);
            return 0;
        }
    }
    printf("Error: File not found for deletion: %s\n", name);
    return -1;
}

int main() {
    printf("Baseline File System (O(n) Linear Search)\n");
    printf("========================================\n\n");
    
    printf("--- Creating Files ---\n");
    create_file("test1.txt", "Hello, World!");
    create_file("test2.txt", "Operating System Concepts with enhanced performance.");
    create_file("notes.txt", "This is a note about hash-table based file systems.");
    create_file("config.txt", "System configuration data");
    create_file("performance_test.txt", "Testing hash table performance improvements");
    
    printf("\n--- Reading Files ---\n");
    read_file("test2.txt");
    read_file("notes.txt");
    read_file("test1.txt");
    
    printf("\n--- Deleting Files ---\n");
    delete_file("test1.txt");
    
    printf("\nBaseline file system completed.\n");
    return 0;
}