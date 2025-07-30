#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MEMORY_SIZE 1024
#define PAGE_SIZE 64
#define NUM_PAGES (MEMORY_SIZE / PAGE_SIZE)

typedef struct {
    int page_number;
    int is_free;
} Page;

Page memory[NUM_PAGES];

void initialize_memory() {
    for (int i = 0; i < NUM_PAGES; i++) {
        memory[i].page_number = i;
        memory[i].is_free = 1;
    }
    printf("Baseline Memory Manager initialized with %d pages\n", NUM_PAGES);
}

// O(n) linear search allocation
int allocate_page() {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (memory[i].is_free) {
            memory[i].is_free = 0;
            return i;
        }
    }
    return -1; // No free pages
}

void free_page(int page_number) {
    if (page_number >= 0 && page_number < NUM_PAGES) {
        memory[page_number].is_free = 1;
    }
}

int main() {
    printf("Baseline Memory Manager (O(n) Linear Search)\n");
    printf("===========================================\n\n");
    
    initialize_memory();
    
    // Allocate same number of pages as enhanced version
    printf("--- Allocating 20 pages ---\n");
    int allocated_pages[20];
    
    for (int i = 0; i < 20; i++) {
        allocated_pages[i] = allocate_page();
        if (allocated_pages[i] < 0) {
            printf("Allocation failed at iteration %d\n", i);
            break;
        }
    }
    
    printf("--- Freeing 4 pages ---\n");
    for (int i = 0; i < 4; i++) {
        free_page(allocated_pages[i]);
    }
    
    printf("\nBaseline memory manager completed.\n");
    return 0;
}