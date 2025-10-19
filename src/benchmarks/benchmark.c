#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

double timespec_to_ns(struct timespec *ts) {
    return ts->tv_sec * 1000000000.0 + ts->tv_nsec;
}

double run_and_measure(const char* program) {
    struct timespec start, end;
    pid_t pid;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    pid = fork();
    if (pid == 0) {
        // Child process - redirect output to /dev/null for cleaner measurement
        freopen("/dev/null", "w", stdout);
        execl(program, program, (char*)NULL);
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
        return -1;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double start_ns = timespec_to_ns(&start);
    double end_ns = timespec_to_ns(&end);
    
    return end_ns - start_ns;
}

int main() {
    printf("Nanosecond Precision Performance Benchmark\n");
    printf("==========================================\n\n");
    
    // Run each test multiple times for accuracy
    const int num_runs = 5;
    double total_enhanced, total_baseline;
    
    printf("🔬 MEMORY MANAGER COMPARISON:\n");
    printf("Running %d iterations each...\n\n", num_runs);
    
    // Enhanced Memory Manager
    total_enhanced = 0;
    for (int i = 0; i < num_runs; i++) {
        double time_ns = run_and_measure("./memory_manager");
        total_enhanced += time_ns;
        printf("Enhanced run %d: %.0f ns\n", i+1, time_ns);
    }
    double avg_enhanced = total_enhanced / num_runs;
    
    printf("\n");
    
    // Baseline Memory Manager  
    total_baseline = 0;
    for (int i = 0; i < num_runs; i++) {
        double time_ns = run_and_measure("./memory_baseline");
        total_baseline += time_ns;
        printf("Baseline run %d: %.0f ns\n", i+1, time_ns);
    }
    double avg_baseline = total_baseline / num_runs;
    
    printf("\n📊 MEMORY MANAGER RESULTS:\n");
    printf("Enhanced (O(1) bitmap):     %.0f ns average\n", avg_enhanced);
    printf("Baseline (O(n) linear):     %.0f ns average\n", avg_baseline);
    printf("Performance improvement:    %.2fx faster\n", avg_baseline / avg_enhanced);
    printf("Time difference:            %.0f ns saved\n\n", avg_baseline - avg_enhanced);
    
    printf("🔬 FILE SYSTEM COMPARISON:\n");
    printf("Running %d iterations each...\n\n", num_runs);
    
    // Enhanced File System
    total_enhanced = 0;
    for (int i = 0; i < num_runs; i++) {
        double time_ns = run_and_measure("./file_system_enhanced");
        total_enhanced += time_ns;
        printf("Enhanced run %d: %.0f ns\n", i+1, time_ns);
    }
    avg_enhanced = total_enhanced / num_runs;
    
    printf("\n");
    
    // Baseline File System
    total_baseline = 0;
    for (int i = 0; i < num_runs; i++) {
        double time_ns = run_and_measure("./filesystem_baseline");
        total_baseline += time_ns;
        printf("Baseline run %d: %.0f ns\n", i+1, time_ns);
    }
    avg_baseline = total_baseline / num_runs;
    
    printf("\n📊 FILE SYSTEM RESULTS:\n");
    printf("Enhanced (O(1) hash table): %.0f ns average\n", avg_enhanced);
    printf("Baseline (O(n) linear):     %.0f ns average\n", avg_baseline);
    printf("Performance improvement:    %.2fx faster\n", avg_baseline / avg_enhanced);
    printf("Time difference:            %.0f ns saved\n\n", avg_baseline - avg_enhanced);
    
    return 0;
}