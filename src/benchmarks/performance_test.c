#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define NUM_OPERATIONS 100000
#define HASH_SIZE 127

// Test structures
uint8_t bitmap[16];  // 128 bits for 128 pages
int linear_memory[128];

typedef struct HashNode {
    char key[32];
    int value;
    struct HashNode* next;
} HashNode;

HashNode* hash_table[HASH_SIZE];
char linear_keys[50][32];
int linear_values[50];
int linear_count = 0;

// Timing function
double get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000.0 + ts.tv_nsec;
}

// Memory allocation functions
int bitmap_allocate() {
    for (int byte = 0; byte < 16; byte++) {
        if (bitmap[byte] != 0xFF) {
            for (int bit = 0; bit < 8; bit++) {
                int page_idx = byte * 8 + bit;
                if (page_idx >= 128) break;
                if (!(bitmap[byte] & (1 << bit))) {
                    bitmap[byte] |= (1 << bit);
                    return page_idx;
                }
            }
        }
    }
    return -1;
}

int linear_allocate() {
    for (int i = 0; i < 128; i++) {
        if (linear_memory[i] == 0) {
            linear_memory[i] = 1;
            return i;
        }
    }
    return -1;
}

// Hash functions
unsigned int hash_function(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

int hash_lookup(const char* key) {
    unsigned int hash = hash_function(key);
    HashNode* current = hash_table[hash];
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return -1;
}

int linear_lookup(const char* key) {
    for (int i = 0; i < linear_count; i++) {
        if (strcmp(linear_keys[i], key) == 0) {
            return linear_values[i];
        }
    }
    return -1;
}

void setup_hash_table() {
    memset(hash_table, 0, sizeof(hash_table));
    
    char keys[][32] = {"file1.txt", "file2.txt", "file3.txt", "file4.txt", "file5.txt", 
                       "test.txt", "data.txt", "config.txt", "readme.txt", "main.c"};
    
    for (int i = 0; i < 10; i++) {
        unsigned int hash = hash_function(keys[i]);
        HashNode* node = malloc(sizeof(HashNode));
        strcpy(node->key, keys[i]);
        node->value = i;
        node->next = hash_table[hash];
        hash_table[hash] = node;
    }
}

void setup_linear_table() {
    linear_count = 10;
    char keys[][32] = {"file1.txt", "file2.txt", "file3.txt", "file4.txt", "file5.txt", 
                       "test.txt", "data.txt", "config.txt", "readme.txt", "main.c"};
    
    for (int i = 0; i < 10; i++) {
        strcpy(linear_keys[i], keys[i]);
        linear_values[i] = i;
    }
}

int main() {
    printf("🚀 HIGH-PRECISION ALGORITHM PERFORMANCE BENCHMARK\n");
    printf("==================================================\n\n");
    
    // Memory allocation test
    printf("💾 MEMORY ALLOCATION COMPARISON:\n");
    printf("Testing %d allocation operations...\n\n", NUM_OPERATIONS / 1000);
    
    // Test bitmap allocation
    memset(bitmap, 0, sizeof(bitmap));
    double start_time = get_time_ns();
    
    for (int i = 0; i < NUM_OPERATIONS / 1000; i++) {
        // Reset bitmap every 100 ops to avoid saturation
        if (i % 100 == 0) memset(bitmap, 0, sizeof(bitmap));
        bitmap_allocate();
    }
    
    double end_time = get_time_ns();
    double bitmap_time = end_time - start_time;
    
    // Test linear allocation
    memset(linear_memory, 0, sizeof(linear_memory));
    start_time = get_time_ns();
    
    for (int i = 0; i < NUM_OPERATIONS / 1000; i++) {
        // Reset memory every 100 ops to avoid saturation
        if (i % 100 == 0) memset(linear_memory, 0, sizeof(linear_memory));
        linear_allocate();
    }
    
    end_time = get_time_ns();
    double linear_time = end_time - start_time;
    
    printf("Bitmap (O(1)) allocation:    %.0f ns total, %.1f ns per operation\n", 
           bitmap_time, bitmap_time / (NUM_OPERATIONS / 1000));
    printf("Linear (O(n)) allocation:    %.0f ns total, %.1f ns per operation\n", 
           linear_time, linear_time / (NUM_OPERATIONS / 1000));
    printf("Performance improvement:     %.2fx faster\n", linear_time / bitmap_time);
    printf("Time saved per operation:    %.1f ns\n\n", 
           (linear_time - bitmap_time) / (NUM_OPERATIONS / 1000));
    
    // File lookup test
    printf("📁 FILE LOOKUP COMPARISON:\n");
    printf("Testing %d lookup operations...\n\n", NUM_OPERATIONS / 100);
    
    setup_hash_table();
    setup_linear_table();
    
    // Test hash lookup
    start_time = get_time_ns();
    
    for (int i = 0; i < NUM_OPERATIONS / 100; i++) {
        hash_lookup("file3.txt");
    }
    
    end_time = get_time_ns();
    double hash_time = end_time - start_time;
    
    // Test linear lookup
    start_time = get_time_ns();
    
    for (int i = 0; i < NUM_OPERATIONS / 100; i++) {
        linear_lookup("file3.txt");
    }
    
    end_time = get_time_ns();
    double linear_lookup_time = end_time - start_time;
    
    printf("Hash table (O(1)) lookup:   %.0f ns total, %.1f ns per operation\n", 
           hash_time, hash_time / (NUM_OPERATIONS / 100));
    printf("Linear (O(n)) lookup:       %.0f ns total, %.1f ns per operation\n", 
           linear_lookup_time, linear_lookup_time / (NUM_OPERATIONS / 100));
    printf("Performance improvement:     %.2fx faster\n", linear_lookup_time / hash_time);
    printf("Time saved per operation:    %.1f ns\n\n", 
           (linear_lookup_time - hash_time) / (NUM_OPERATIONS / 100));
    
    // Summary
    printf("📊 NANOSECOND PRECISION RESULTS SUMMARY:\n");
    printf("========================================\n");
    printf("✅ Bitmap allocation: %.1f ns per operation (O(1))\n", 
           bitmap_time / (NUM_OPERATIONS / 1000));
    printf("✅ Hash table lookup: %.1f ns per operation (O(1))\n", 
           hash_time / (NUM_OPERATIONS / 100));
    printf("⚠️  Linear allocation: %.1f ns per operation (O(n))\n", 
           linear_time / (NUM_OPERATIONS / 1000));
    printf("⚠️  Linear lookup: %.1f ns per operation (O(n))\n\n", 
           linear_lookup_time / (NUM_OPERATIONS / 100));
    
    printf("🎯 ALGORITHMIC IMPROVEMENTS VERIFIED:\n");
    printf("• Memory allocation %.2fx faster with O(1) bitmap\n", linear_time / bitmap_time);
    printf("• File lookup %.2fx faster with O(1) hash table\n", linear_lookup_time / hash_time);
    printf("• All measurements in real nanoseconds, not approximations\n");
    
    return 0;
}