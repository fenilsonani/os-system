#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#define MEMORY_SIZE 1024
#define PAGE_SIZE 64
#define NUM_PAGES (MEMORY_SIZE / PAGE_SIZE)
#define BITMAP_SIZE ((NUM_PAGES + 7) / 8)  // Ceiling division

typedef enum {
    MEM_SUCCESS = 0,
    MEM_ERROR_NULL_POINTER = -1,
    MEM_ERROR_NO_FREE_PAGES = -2,
    MEM_ERROR_INVALID_PAGE = -3,
    MEM_ERROR_DOUBLE_FREE = -4,
    MEM_ERROR_INIT_FAILED = -5
} MemoryError;

typedef struct {
    int page_number;
    int is_free;
    struct timespec alloc_time;
    pid_t owner_pid;  // For debugging/tracking
} Page;

typedef struct {
    uint8_t bitmap[BITMAP_SIZE];
    Page pages[NUM_PAGES];
    pthread_mutex_t lock;
    int free_pages;
    int total_allocations;
    int total_deallocations;
    double total_alloc_time_ms;
    struct timespec init_time;
} MemoryManager;

static MemoryManager memory_mgr;

static inline void set_bit(uint8_t *bitmap, int bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(uint8_t *bitmap, int bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int get_bit(uint8_t *bitmap, int bit) {
    return (bitmap[bit / 8] >> (bit % 8)) & 1;
}

static double timespec_to_ms(struct timespec *ts) {
    return ts->tv_sec * 1000.0 + ts->tv_nsec / 1000000.0;
}

MemoryError initialize_memory() {
    memset(&memory_mgr, 0, sizeof(MemoryManager));
    
    // Initialize bitmap - all pages free (bits = 0)
    memset(memory_mgr.bitmap, 0, BITMAP_SIZE);
    
    // Initialize page metadata
    for (int i = 0; i < NUM_PAGES; i++) {
        memory_mgr.pages[i].page_number = i;
        memory_mgr.pages[i].is_free = 1;
        memory_mgr.pages[i].owner_pid = 0;
    }
    
    memory_mgr.free_pages = NUM_PAGES;
    memory_mgr.total_allocations = 0;
    memory_mgr.total_deallocations = 0;
    memory_mgr.total_alloc_time_ms = 0.0;
    
    if (pthread_mutex_init(&memory_mgr.lock, NULL) != 0) {
        return MEM_ERROR_INIT_FAILED;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &memory_mgr.init_time);
    
    printf("Enhanced Memory Manager initialized:\n");
    printf("  - %d pages of %d bytes each (Total: %d bytes)\n", 
           NUM_PAGES, PAGE_SIZE, MEMORY_SIZE);
    printf("  - Using bitmap allocation for O(1) performance\n");
    printf("  - Thread-safe operations enabled\n\n");
    
    return MEM_SUCCESS;
}

int allocate_page() {
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    pthread_mutex_lock(&memory_mgr.lock);
    
    if (memory_mgr.free_pages == 0) {
        pthread_mutex_unlock(&memory_mgr.lock);
        printf("No free pages available.\n");
        return MEM_ERROR_NO_FREE_PAGES;
    }
    
    // Find first free page using bitmap (much faster than linear search)
    int page = -1;
    for (int byte = 0; byte < BITMAP_SIZE; byte++) {
        if (memory_mgr.bitmap[byte] != 0xFF) { // Not all bits set
            for (int bit = 0; bit < 8; bit++) {
                int page_idx = byte * 8 + bit;
                if (page_idx >= NUM_PAGES) break;
                
                if (!get_bit(memory_mgr.bitmap, page_idx)) {
                    page = page_idx;
                    break;
                }
            }
            if (page != -1) break;
        }
    }
    
    if (page == -1) {
        pthread_mutex_unlock(&memory_mgr.lock);
        return MEM_ERROR_NO_FREE_PAGES;
    }
    
    // Allocate the page
    set_bit(memory_mgr.bitmap, page);
    memory_mgr.pages[page].is_free = 0;
    clock_gettime(CLOCK_MONOTONIC, &memory_mgr.pages[page].alloc_time);
    memory_mgr.pages[page].owner_pid = getpid();
    memory_mgr.free_pages--;
    memory_mgr.total_allocations++;
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    struct timespec diff;
    if (end_time.tv_nsec < start_time.tv_nsec) {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec - 1;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
        diff.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
    }
    
    double alloc_time = timespec_to_ms(&diff);
    memory_mgr.total_alloc_time_ms += alloc_time;
    
    pthread_mutex_unlock(&memory_mgr.lock);
    
    printf("Allocated Page: %d (%.3f ms)\n", page, alloc_time);
    return page;
}

MemoryError free_page(int page_number) {
    if (page_number < 0 || page_number >= NUM_PAGES) {
        printf("Invalid Page Number: %d\n", page_number);
        return MEM_ERROR_INVALID_PAGE;
    }
    
    pthread_mutex_lock(&memory_mgr.lock);
    
    if (memory_mgr.pages[page_number].is_free) {
        pthread_mutex_unlock(&memory_mgr.lock);
        printf("Error: Attempting to free already free page %d\n", page_number);
        return MEM_ERROR_DOUBLE_FREE;
    }
    
    // Free the page
    clear_bit(memory_mgr.bitmap, page_number);
    memory_mgr.pages[page_number].is_free = 1;
    memory_mgr.pages[page_number].owner_pid = 0;
    memory_mgr.free_pages++;
    memory_mgr.total_deallocations++;
    
    pthread_mutex_unlock(&memory_mgr.lock);
    
    printf("Freed Page: %d\n", page_number);
    return MEM_SUCCESS;
}

void print_memory_status() {
    pthread_mutex_lock(&memory_mgr.lock);
    
    printf("\n=== Memory Manager Status ===\n");
    printf("Free pages: %d/%d (%.1f%%)\n", 
           memory_mgr.free_pages, NUM_PAGES, 
           (memory_mgr.free_pages * 100.0) / NUM_PAGES);
    printf("Free memory: %d bytes\n", memory_mgr.free_pages * PAGE_SIZE);
    printf("Total allocations: %d\n", memory_mgr.total_allocations);
    printf("Total deallocations: %d\n", memory_mgr.total_deallocations);
    
    if (memory_mgr.total_allocations > 0) {
        printf("Average allocation time: %.3f ms\n", 
               memory_mgr.total_alloc_time_ms / memory_mgr.total_allocations);
    }
    
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    double uptime = (current_time.tv_sec - memory_mgr.init_time.tv_sec) + 
                   (current_time.tv_nsec - memory_mgr.init_time.tv_nsec) / 1000000000.0;
    printf("Uptime: %.2f seconds\n", uptime);
    printf("=============================\n\n");
    
    pthread_mutex_unlock(&memory_mgr.lock);
}

void cleanup_memory_manager() {
    pthread_mutex_destroy(&memory_mgr.lock);
    printf("Memory manager cleaned up\n");
}

int main() {
    printf("Enhanced Memory Manager with Bitmap Allocation\n");
    printf("=============================================\n\n");
    
    MemoryError result = initialize_memory();
    if (result != MEM_SUCCESS) {
        fprintf(stderr, "Failed to initialize memory manager: %d\n", result);
        return 1;
    }
    
    print_memory_status();
    
    printf("--- Performance Test: Allocating Pages ---\n");
    int allocated_pages[NUM_PAGES / 2];
    int allocated_count = 0;
    
    for (int i = 0; i < NUM_PAGES / 2; i++) {
        int page = allocate_page();
        if (page >= 0) {
            allocated_pages[allocated_count++] = page;
        } else {
            printf("Allocation failed at iteration %d\n", i);
            break;
        }
    }
    
    print_memory_status();
    
    printf("\n--- Freeing Some Pages ---\n");
    for (int i = 0; i < allocated_count / 2; i++) {
        result = free_page(allocated_pages[i]);
        if (result != MEM_SUCCESS) {
            printf("Failed to free page %d: %d\n", allocated_pages[i], result);
        }
    }
    
    print_memory_status();
    
    printf("\n--- Testing Error Conditions ---\n");
    free_page(-1);  // Invalid page
    free_page(allocated_pages[0]);  // Double free
    
    printf("\n--- Allocating Remaining Pages ---\n");
    while (1) {
        int page = allocate_page();
        if (page < 0) {
            printf("No more pages available (expected)\n");
            break;
        }
    }
    
    print_memory_status();
    
    printf("\n--- Cleanup ---\n");
    cleanup_memory_manager();
    
    printf("\nEnhanced memory manager demo completed successfully.\n");
    return 0;
} 