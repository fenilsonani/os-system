#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int create_file(const char* name, const char* data) {
    int slot = -1;

    // First, check for existing file with the same name
    for (int i = 0; i < file_count; i++) {
        if (!file_system[i].is_deleted && strcmp(file_system[i].filename, name) == 0) {
            printf("Error: File already exists: %s\n", name);
            return -1;
        }
    }
    
    // Next, try to find a deleted slot to reuse
    for (int i = 0; i < file_count; i++) {
        if (file_system[i].is_deleted) {
            slot = i;
            printf("Reusing deleted slot %d for file %s\n", slot, name);
            break;
        }
    }

    // If no deleted slot found, try appending if space available
    if (slot == -1) {
        if (file_count < MAX_FILES) {
            slot = file_count;
            file_count++; // Only increment count when using a new slot
        } else {
            printf("Error: File system is full. Cannot create file %s\n", name);
            return -1;
        }
    }

    // Check if data exceeds max filesize (optional but good practice)
    size_t data_len = strlen(data);
    if (data_len >= MAX_FILESIZE) {
        fprintf(stderr, "Warning: Data for file '%s' truncated to %d bytes.\n", name, MAX_FILESIZE - 1);
        // No need to actually change data_len if strncpy handles null termination
    }

    // Populate the slot
    strncpy(file_system[slot].filename, name, MAX_FILENAME - 1);
    file_system[slot].filename[MAX_FILENAME - 1] = '\0'; // Ensure null termination
    strncpy(file_system[slot].data, data, MAX_FILESIZE - 1);
    file_system[slot].data[MAX_FILESIZE - 1] = '\0'; // Ensure null termination
    file_system[slot].size = strlen(file_system[slot].data); // Use length after potential truncation
    file_system[slot].is_deleted = 0;

    printf("Created File: %s in slot %d (Size: %d bytes)\n", 
           file_system[slot].filename, slot, file_system[slot].size);
    return slot; // Return slot index on success
}

int read_file(const char* name) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_system[i].filename, name) == 0 && !file_system[i].is_deleted) {
            printf("Reading File: %s\nSize: %d bytes\nContent: %s\n", 
                   name, file_system[i].size, file_system[i].data);
            return 0;
        }
    }
    printf("File not found: %s\n", name);
    return -1;
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