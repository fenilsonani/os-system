#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define MAX_PROCESSES 1024
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define MAX_BURST_TIME 10

typedef enum {
    SCHED_SUCCESS = 0,
    SCHED_ERROR_NULL_POINTER = -1,
    SCHED_ERROR_QUEUE_FULL = -2,
    SCHED_ERROR_QUEUE_EMPTY = -3,
    SCHED_ERROR_INVALID_PRIORITY = -4,
    SCHED_ERROR_MEMORY_ALLOCATION = -5
} SchedulerError;

typedef struct {
    int process_id;
    int priority;
    int burst_time;
    struct timespec arrival_time;
    struct timespec start_time;
} Process;

typedef struct {
    Process* heap;
    int capacity;
    int size;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    int shutdown;
    int total_processed;
    double total_wait_time;
} PriorityQueue;

static void timespec_diff(struct timespec *start, struct timespec *stop, struct timespec *result) {
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}

static double timespec_to_ms(struct timespec *ts) {
    return ts->tv_sec * 1000.0 + ts->tv_nsec / 1000000.0;
}

static void heap_up(PriorityQueue* q, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (q->heap[index].priority <= q->heap[parent].priority) break;
        
        Process temp = q->heap[index];
        q->heap[index] = q->heap[parent];
        q->heap[parent] = temp;
        index = parent;
    }
}

static void heap_down(PriorityQueue* q, int index) {
    while (1) {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        
        if (left < q->size && q->heap[left].priority > q->heap[largest].priority)
            largest = left;
        if (right < q->size && q->heap[right].priority > q->heap[largest].priority)
            largest = right;
            
        if (largest == index) break;
        
        Process temp = q->heap[index];
        q->heap[index] = q->heap[largest];
        q->heap[largest] = temp;
        index = largest;
    }
}

SchedulerError init_priority_queue(PriorityQueue* q, int capacity) {
    if (!q) return SCHED_ERROR_NULL_POINTER;
    
    q->heap = malloc(capacity * sizeof(Process));
    if (!q->heap) return SCHED_ERROR_MEMORY_ALLOCATION;
    
    q->capacity = capacity;
    q->size = 0;
    q->shutdown = 0;
    q->total_processed = 0;
    q->total_wait_time = 0.0;
    
    if (pthread_mutex_init(&q->lock, NULL) != 0) {
        free(q->heap);
        return SCHED_ERROR_MEMORY_ALLOCATION;
    }
    
    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&q->lock);
        free(q->heap);
        return SCHED_ERROR_MEMORY_ALLOCATION;
    }
    
    return SCHED_SUCCESS;
}

SchedulerError enqueue(PriorityQueue* q, Process p) {
    if (!q) return SCHED_ERROR_NULL_POINTER;
    if (p.priority < MIN_PRIORITY || p.priority > MAX_PRIORITY) 
        return SCHED_ERROR_INVALID_PRIORITY;
    
    pthread_mutex_lock(&q->lock);
    
    if (q->size >= q->capacity) {
        pthread_mutex_unlock(&q->lock);
        return SCHED_ERROR_QUEUE_FULL;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &p.arrival_time);
    q->heap[q->size] = p;
    heap_up(q, q->size);
    q->size++;
    
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
    
    return SCHED_SUCCESS;
}

SchedulerError dequeue(PriorityQueue* q, Process* p) {
    if (!q || !p) return SCHED_ERROR_NULL_POINTER;
    
    pthread_mutex_lock(&q->lock);
    
    while (q->size == 0 && !q->shutdown) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    
    if (q->shutdown && q->size == 0) {
        pthread_mutex_unlock(&q->lock);
        return SCHED_ERROR_QUEUE_EMPTY;
    }
    
    *p = q->heap[0];
    q->heap[0] = q->heap[q->size - 1];
    q->size--;
    heap_down(q, 0);
    
    clock_gettime(CLOCK_MONOTONIC, &p->start_time);
    
    struct timespec wait_time;
    timespec_diff(&p->arrival_time, &p->start_time, &wait_time);
    q->total_wait_time += timespec_to_ms(&wait_time);
    q->total_processed++;
    
    pthread_mutex_unlock(&q->lock);
    return SCHED_SUCCESS;
}

void shutdown_queue(PriorityQueue* q) {
    if (!q) return;
    
    pthread_mutex_lock(&q->lock);
    q->shutdown = 1;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

void cleanup_priority_queue(PriorityQueue* q) {
    if (!q) return;
    
    if (q->heap) {
        free(q->heap);
        q->heap = NULL;
    }
    
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
}

void* scheduler(void* arg) {
    PriorityQueue* q = (PriorityQueue*)arg;
    printf("[Scheduler] Thread started with enhanced priority queue\n");
    
    while (1) {
        Process p;
        SchedulerError result = dequeue(q, &p);
        
        if (result == SCHED_ERROR_QUEUE_EMPTY) {
            printf("[Scheduler] Shutdown requested, exiting\n");
            break;
        }
        
        if (result == SCHED_SUCCESS) {
            struct timespec wait_time;
            timespec_diff(&p.arrival_time, &p.start_time, &wait_time);
            double wait_ms = timespec_to_ms(&wait_time);
            
            printf("[Scheduler] Executing Process ID: %d (Priority: %d, Burst: %d, Wait: %.2fms)\n", 
                   p.process_id, p.priority, p.burst_time, wait_ms);
            
            usleep(p.burst_time * 100000); // Simulate work (100ms per burst unit)
            
            printf("[Scheduler] Process ID: %d completed\n", p.process_id);
        }
    }
    
    return NULL;
}

void print_scheduler_stats(PriorityQueue* q) {
    if (!q) return;
    
    pthread_mutex_lock(&q->lock);
    printf("\n=== Scheduler Statistics ===\n");
    printf("Total processes handled: %d\n", q->total_processed);
    if (q->total_processed > 0) {
        printf("Average wait time: %.2f ms\n", q->total_wait_time / q->total_processed);
    }
    printf("Queue size: %d/%d\n", q->size, q->capacity);
    printf("===========================\n\n");
    pthread_mutex_unlock(&q->lock);
}

int main() {
    srand(time(NULL));
    
    printf("Enhanced Process Scheduler with Heap-based Priority Queue\n");
    printf("========================================================\n\n");

    PriorityQueue q;
    SchedulerError result = init_priority_queue(&q, MAX_PROCESSES);
    if (result != SCHED_SUCCESS) {
        fprintf(stderr, "Failed to initialize priority queue: %d\n", result);
        return 1;
    }

    pthread_t sched_thread;
    if (pthread_create(&sched_thread, NULL, scheduler, &q) != 0) {
        perror("Failed to create scheduler thread");
        cleanup_priority_queue(&q);
        return 1;
    }

    printf("[Main] Adding processes to enhanced scheduler...\n");
    
    for (int i = 1; i <= 10; i++) {
        Process p = { 
            .process_id = i, 
            .priority = (rand() % MAX_PRIORITY) + 1, 
            .burst_time = (rand() % MAX_BURST_TIME) + 1 
        };
        
        result = enqueue(&q, p);
        if (result == SCHED_SUCCESS) {
            printf("[Main] Added Process ID: %d (Priority: %d, Burst: %d)\n", 
                   p.process_id, p.priority, p.burst_time);
        } else {
            fprintf(stderr, "[Main] Failed to add process %d: %d\n", i, result);
        }
        
        usleep(500000); // 0.5 second delay
    }

    printf("\n[Main] All processes added. Waiting 3 seconds for completion...\n");
    sleep(3);
    
    print_scheduler_stats(&q);
    
    printf("[Main] Shutting down scheduler...\n");
    shutdown_queue(&q);
    
    pthread_join(sched_thread, NULL);
    cleanup_priority_queue(&q);

    printf("[Main] Enhanced scheduler demo completed successfully.\n");
    return 0;
} 