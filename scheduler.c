#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int process_id;
    int priority;
    int burst_time;
} Process;

typedef struct ProcessNode {
    Process process;
    struct ProcessNode* next;
} ProcessNode;

typedef struct {
    ProcessNode* front;
    pthread_mutex_t lock;
} PriorityQueue;

void enqueue(PriorityQueue* q, Process p) {
    ProcessNode* newNode = (ProcessNode*)malloc(sizeof(ProcessNode));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new process node");
        return;
    }
    newNode->process = p;
    newNode->next = NULL;

    pthread_mutex_lock(&q->lock);
    
    if (q->front == NULL || p.priority > q->front->process.priority) {
        newNode->next = q->front;
        q->front = newNode;
    } else {
        ProcessNode* current = q->front;
        while (current->next != NULL && current->next->process.priority >= p.priority) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
    
    pthread_mutex_unlock(&q->lock);
}

Process dequeue(PriorityQueue* q) {
    pthread_mutex_lock(&q->lock);
    if (q->front == NULL) {
        pthread_mutex_unlock(&q->lock);
        Process empty = { -1, -1, -1 };
        return empty;
    }
    ProcessNode* temp = q->front;
    q->front = q->front->next;
    pthread_mutex_unlock(&q->lock);
    
    Process p = temp->process;
    free(temp);
    return p;
}

void* scheduler(void* arg) {
    PriorityQueue* q = (PriorityQueue*)arg;
    printf("Scheduler thread started.\n");
    while (1) {
        Process p = dequeue(q);
        if (p.process_id != -1) {
            printf("[Scheduler] Executing Process ID: %d (Priority: %d, Burst: %d)\n", 
                   p.process_id, p.priority, p.burst_time);
            sleep(p.burst_time);
            printf("[Scheduler] Process ID: %d completed.\n", p.process_id);
        } else {
            usleep(100000);
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    PriorityQueue q;
    q.front = NULL;
    pthread_mutex_init(&q.lock, NULL);

    pthread_t sched_thread;
    if (pthread_create(&sched_thread, NULL, scheduler, &q) != 0) {
        perror("Failed to create scheduler thread");
        return 1;
    }

    printf("Main thread adding processes...\n");
    for (int i = 1; i <= 5; i++) {
        Process p = { i, rand() % 10 + 1, rand() % 3 + 1 };
        printf("[Main] Adding Process ID: %d (Priority: %d, Burst: %d)\n", 
               p.process_id, p.priority, p.burst_time);
        enqueue(&q, p);
        sleep(1);
    }

    printf("[Main] Waiting for scheduler thread (will run indefinitely)...\n");
    pthread_join(sched_thread, NULL);

    pthread_mutex_destroy(&q.lock);
    ProcessNode* current = q.front;
    while (current != NULL) {
        ProcessNode* temp = current;
        current = current->next;
        free(temp);
    }

    printf("[Main] Exiting.\n");
    return 0;
} 