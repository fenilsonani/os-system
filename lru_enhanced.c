#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define NUM_FRAMES 4
#define HASH_TABLE_SIZE 16

typedef enum {
    LRU_SUCCESS = 0,
    LRU_ERROR_NULL_POINTER = -1,
    LRU_ERROR_INVALID_PAGE = -2,
    LRU_ERROR_INIT_FAILED = -3
} LRUError;

typedef struct PageNode {
    int page_number;
    int frame_number;
    struct timespec access_time;
    struct PageNode* prev;
    struct PageNode* next;
    struct PageNode* hash_next;
} PageNode;

typedef struct {
    PageNode* hash_table[HASH_TABLE_SIZE];
    PageNode* head;  // Most recently used
    PageNode* tail;  // Least recently used
    PageNode frames[NUM_FRAMES];
    pthread_mutex_t lock;
    int current_time;
    int page_faults;
    int page_hits;
    int total_accesses;
    double total_access_time_ms;
} LRUCache;

static LRUCache lru_cache;

static unsigned int hash_function(int page_number) {
    return page_number % HASH_TABLE_SIZE;
}

static double timespec_to_ms(struct timespec *ts) {
    return ts->tv_sec * 1000.0 + ts->tv_nsec / 1000000.0;
}

LRUError init_lru_cache() {
    memset(&lru_cache, 0, sizeof(LRUCache));
    
    // Initialize doubly linked list
    lru_cache.head = NULL;
    lru_cache.tail = NULL;
    
    // Initialize hash table
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        lru_cache.hash_table[i] = NULL;
    }
    
    // Initialize frame nodes
    for (int i = 0; i < NUM_FRAMES; i++) {
        lru_cache.frames[i].page_number = -1;
        lru_cache.frames[i].frame_number = i;
        lru_cache.frames[i].prev = NULL;
        lru_cache.frames[i].next = NULL;
        lru_cache.frames[i].hash_next = NULL;
    }
    
    if (pthread_mutex_init(&lru_cache.lock, NULL) != 0) {
        return LRU_ERROR_INIT_FAILED;
    }
    
    printf("Enhanced LRU Page Replacement with O(1) Operations\n");
    printf("================================================\n\n");
    printf("  - Hash table + doubly-linked list for O(1) access\n");
    printf("  - %d frames available\n", NUM_FRAMES);
    printf("  - Thread-safe operations\n\n");
    
    return LRU_SUCCESS;
}

static void remove_from_list(PageNode* node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        lru_cache.head = node->next;
    }
    
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        lru_cache.tail = node->prev;
    }
    
    node->prev = node->next = NULL;
}

static void add_to_head(PageNode* node) {
    node->next = lru_cache.head;
    node->prev = NULL;
    
    if (lru_cache.head) {
        lru_cache.head->prev = node;
    }
    lru_cache.head = node;
    
    if (!lru_cache.tail) {
        lru_cache.tail = node;
    }
}

static void move_to_head(PageNode* node) {
    remove_from_list(node);
    add_to_head(node);
}

static PageNode* find_page_in_hash(int page_number) {
    unsigned int hash = hash_function(page_number);
    PageNode* current = lru_cache.hash_table[hash];
    
    while (current) {
        if (current->page_number == page_number) {
            return current;
        }
        current = current->hash_next;
    }
    return NULL;
}

static void add_to_hash(PageNode* node) {
    unsigned int hash = hash_function(node->page_number);
    node->hash_next = lru_cache.hash_table[hash];
    lru_cache.hash_table[hash] = node;
}

static void remove_from_hash(PageNode* node) {
    unsigned int hash = hash_function(node->page_number);
    PageNode* current = lru_cache.hash_table[hash];
    PageNode* prev = NULL;
    
    while (current) {
        if (current == node) {
            if (prev) {
                prev->hash_next = current->hash_next;
            } else {
                lru_cache.hash_table[hash] = current->hash_next;
            }
            node->hash_next = NULL;
            return;
        }
        prev = current;
        current = current->hash_next;
    }
}

static PageNode* get_lru_frame() {
    return lru_cache.tail;
}

LRUError access_page(int page_number) {
    if (page_number < 0) return LRU_ERROR_INVALID_PAGE;
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    pthread_mutex_lock(&lru_cache.lock);
    
    lru_cache.current_time++;
    printf("\nAccessing page %d at time %d\n", page_number, lru_cache.current_time);
    
    PageNode* node = find_page_in_hash(page_number);
    
    if (node) {
        // Page hit - move to head
        lru_cache.page_hits++;
        clock_gettime(CLOCK_MONOTONIC, &node->access_time);
        move_to_head(node);
        printf("Page %d found in frame %d (HIT)\n", page_number, node->frame_number);
    } else {
        // Page fault
        lru_cache.page_faults++;
        printf("Page %d not found in memory (FAULT)\n", page_number);
        
        PageNode* frame_to_use = NULL;
        
        // Find empty frame first
        for (int i = 0; i < NUM_FRAMES; i++) {
            if (lru_cache.frames[i].page_number == -1) {
                frame_to_use = &lru_cache.frames[i];
                printf("Loading page %d into empty frame %d\n", page_number, i);
                break;
            }
        }
        
        if (!frame_to_use) {
            // Use LRU replacement
            frame_to_use = get_lru_frame();
            printf("Replacing page %d in frame %d using LRU policy\n", 
                   frame_to_use->page_number, frame_to_use->frame_number);
            
            // Remove old page from hash table
            remove_from_hash(frame_to_use);
            remove_from_list(frame_to_use);
        }
        
        // Update frame with new page
        frame_to_use->page_number = page_number;
        clock_gettime(CLOCK_MONOTONIC, &frame_to_use->access_time);
        
        // Add to hash table and move to head
        add_to_hash(frame_to_use);
        add_to_head(frame_to_use);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    struct timespec diff;
    if (end_time.tv_nsec < start_time.tv_nsec) {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec - 1;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
    }
    
    double access_time = timespec_to_ms(&diff);
    lru_cache.total_access_time_ms += access_time;
    lru_cache.total_accesses++;
    
    pthread_mutex_unlock(&lru_cache.lock);
    
    printf("Access time: %.3f ms\n", access_time);
    return LRU_SUCCESS;
}

void print_page_table() {
    pthread_mutex_lock(&lru_cache.lock);
    
    printf("\n=== Current Page Table State ===\n");
    printf("| %-6s | %-10s | %-15s |\n", "Frame", "Page", "Position");
    printf("-----------------------------------\n");
    
    PageNode* current = lru_cache.head;
    int position = 1;
    
    while (current) {
        printf("| %-6d | %-10d | %-15s |\n", 
               current->frame_number, current->page_number,
               position == 1 ? "Most Recent" : 
               (current == lru_cache.tail ? "Least Recent" : "Middle"));
        current = current->next;
        position++;
    }
    
    printf("===================================\n");
    
    pthread_mutex_unlock(&lru_cache.lock);
}

void print_lru_statistics() {
    pthread_mutex_lock(&lru_cache.lock);
    
    printf("\n=== LRU Cache Statistics ===\n");
    printf("Total accesses: %d\n", lru_cache.total_accesses);
    printf("Page hits: %d (%.2f%%)\n", lru_cache.page_hits, 
           lru_cache.total_accesses > 0 ? 
           (lru_cache.page_hits * 100.0) / lru_cache.total_accesses : 0.0);
    printf("Page faults: %d (%.2f%%)\n", lru_cache.page_faults,
           lru_cache.total_accesses > 0 ? 
           (lru_cache.page_faults * 100.0) / lru_cache.total_accesses : 0.0);
    
    if (lru_cache.total_accesses > 0) {
        printf("Average access time: %.3f ms\n", 
               lru_cache.total_access_time_ms / lru_cache.total_accesses);
    }
    printf("===========================\n\n");
    
    pthread_mutex_unlock(&lru_cache.lock);
}

void cleanup_lru_cache() {
    pthread_mutex_destroy(&lru_cache.lock);
    printf("LRU cache cleaned up\n");
}

int main() {
    if (init_lru_cache() != LRU_SUCCESS) {
        fprintf(stderr, "Failed to initialize LRU cache\n");
        return 1;
    }
    
    print_page_table();
    
    // Simulate page access pattern
    int access_pattern[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5, 6, 1, 7, 8};
    int pattern_length = sizeof(access_pattern) / sizeof(access_pattern[0]);
    
    printf("Simulating enhanced page access pattern: ");
    for (int i = 0; i < pattern_length; i++) {
        printf("%d ", access_pattern[i]);
    }
    printf("\n");
    
    for (int i = 0; i < pattern_length; i++) {
        if (access_page(access_pattern[i]) != LRU_SUCCESS) {
            printf("Error accessing page %d\n", access_pattern[i]);
        }
        print_page_table();
    }
    
    print_lru_statistics();
    cleanup_lru_cache();
    
    printf("Enhanced LRU page replacement demo completed successfully.\n");
    return 0;
}