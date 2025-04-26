#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    printf("Memory initialized with %d pages of %d bytes each (Total: %d bytes)\n", 
           NUM_PAGES, PAGE_SIZE, MEMORY_SIZE);
}

int allocate_page() {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (memory[i].is_free) {
            memory[i].is_free = 0;
            printf("Allocated Page: %d\n", i);
            return i;
        }
    }
    printf("No free pages available.\n");
    return -1;
}

void free_page(int page_number) {
    if (page_number >= 0 && page_number < NUM_PAGES) {
        memory[page_number].is_free = 1;
        printf("Freed Page: %d\n", page_number);
    } else {
        printf("Invalid Page Number: %d\n", page_number);
    }
}

void print_memory_status() {
    int free_pages = 0;
    for (int i = 0; i < NUM_PAGES; i++) {
        if (memory[i].is_free) {
            free_pages++;
        }
    }
    printf("Memory Status: %d/%d pages free (%d bytes free)\n", 
           free_pages, NUM_PAGES, free_pages * PAGE_SIZE);
}

int main() {
    initialize_memory();
    print_memory_status();
    
    printf("\n--- Allocating Pages ---\n");
    int p1 = allocate_page();
    if (p1 == -1) {
        fprintf(stderr, "Initial allocation failed! Exiting.\n");
        return 1;
    }
    allocate_page();
    allocate_page();
    
    print_memory_status();
    
    printf("\n--- Freeing Page ---\n");
    free_page(p1);
    
    print_memory_status();
    
    printf("\n--- Allocating More Pages ---\n");
    allocate_page();
    
    print_memory_status();
    
    printf("\nMemory manager demo finished.\n");
    return 0;
} 