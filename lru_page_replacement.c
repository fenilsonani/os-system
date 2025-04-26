#include <stdio.h>
#include <stdlib.h>

#define NUM_FRAMES 4

typedef struct {
    int page_number;
    int last_access_time;
    int valid;
} PageFrame;

PageFrame page_table[NUM_FRAMES];
int current_time = 0;
int page_faults = 0;
int page_hits = 0;

void initialize_page_table() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        page_table[i].page_number = -1;
        page_table[i].last_access_time = -1;
        page_table[i].valid = 0;
    }
    
    current_time = 0;
    page_faults = 0;
    page_hits = 0;
    printf("Page table initialized with %d frames\n", NUM_FRAMES);
}

void print_page_table() {
    printf("\nCurrent Page Table State:\n");
    printf("------------------------------------------\n");
    printf("| %-10s | %-15s | %-10s |\n", "Frame", "Page Number", "Last Access");
    printf("------------------------------------------\n");
    
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (page_table[i].valid) {
            printf("| %-10d | %-15d | %-10d |\n", 
                   i, page_table[i].page_number, page_table[i].last_access_time);
        } else {
            printf("| %-10d | %-15s | %-10s |\n", i, "Empty", "N/A");
        }
    }
    printf("------------------------------------------\n");
}

int find_page(int page_number) {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (page_table[i].valid && page_table[i].page_number == page_number)
            return i;
    }
    return -1;
}

int find_empty_frame() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (!page_table[i].valid)
            return i;
    }
    return -1;
}

int get_lru_frame() {
    int lru_index = 0;
    int min_time = page_table[0].last_access_time;
    
    for (int i = 1; i < NUM_FRAMES; i++) {
        if (page_table[i].last_access_time < min_time) {
            min_time = page_table[i].last_access_time;
            lru_index = i;
        }
    }
    
    return lru_index;
}

void access_page(int page_number) {
    current_time++;
    printf("\nAccessing page %d at time %d\n", page_number, current_time);
    
    int frame_index = find_page(page_number);
    
    if (frame_index != -1) {
        // Page hit
        page_table[frame_index].last_access_time = current_time;
        page_hits++;
        printf("Page %d found in frame %d (HIT)\n", page_number, frame_index);
    } else {
        // Page fault
        page_faults++;
        printf("Page %d not found in memory (FAULT)\n", page_number);
        
        // Look for an empty frame
        int empty_frame = find_empty_frame();
        
        if (empty_frame != -1) {
            // Empty frame available
            frame_index = empty_frame;
            printf("Loading page %d into empty frame %d\n", page_number, frame_index);
        } else {
            // No empty frames, use LRU replacement
            frame_index = get_lru_frame();
            printf("Replacing page %d in frame %d using LRU policy\n", 
                   page_table[frame_index].page_number, frame_index);
        }
        
        // Update page table
        page_table[frame_index].page_number = page_number;
        page_table[frame_index].last_access_time = current_time;
        page_table[frame_index].valid = 1;
    }
    
    print_page_table();
}

void print_statistics() {
    printf("\nPage Replacement Statistics:\n");
    printf("------------------------------------------\n");
    printf("Total Memory Accesses: %d\n", current_time);
    printf("Page Hits: %d (%.2f%%)\n", page_hits, (float)page_hits/current_time * 100);
    printf("Page Faults: %d (%.2f%%)\n", page_faults, (float)page_faults/current_time * 100);
    printf("------------------------------------------\n");
}

int main() {
    printf("LRU Page Replacement Algorithm Simulation\n");
    printf("=========================================\n\n");
    
    initialize_page_table();
    print_page_table();
    
    // Simulate page access pattern
    int access_pattern[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    int pattern_length = sizeof(access_pattern) / sizeof(access_pattern[0]);
    
    printf("\nSimulating page access pattern: ");
    for (int i = 0; i < pattern_length; i++) {
        printf("%d ", access_pattern[i]);
    }
    printf("\n");
    
    for (int i = 0; i < pattern_length; i++) {
        access_page(access_pattern[i]);
    }
    
    print_statistics();
    
    return 0;
} 