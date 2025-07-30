#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Define the struct for collecting metrics
typedef struct {
    double scheduling_latency_total; // Changed name for clarity
    double memory_allocation_time_total; // Changed name for clarity
    int file_operations_count; // Changed name for clarity
    int total_operations_count; // Changed name for clarity
    time_t start_time;
} MetricsCollector;

// Initialize the metrics collector struct
void initialize_metrics(MetricsCollector* collector) {
    if (collector == NULL) return;
    collector->scheduling_latency_total = 0.0;
    collector->memory_allocation_time_total = 0.0;
    collector->file_operations_count = 0;
    collector->total_operations_count = 0;
    collector->start_time = time(NULL);
    
    printf("Metrics collector initialized\n");
}

// Record scheduling latency
void record_scheduling_latency(MetricsCollector* collector, double latency) {
    if (collector == NULL) return;
    collector->scheduling_latency_total += latency;
    collector->total_operations_count++;
    printf("Recorded scheduling latency: %.2f ms\n", latency);
}

// Record memory allocation time
void record_memory_allocation(MetricsCollector* collector, double allocation_time) {
    if (collector == NULL) return;
    collector->memory_allocation_time_total += allocation_time;
    collector->total_operations_count++;
    printf("Recorded memory allocation time: %.2f ms\n", allocation_time);
}

// Increment file operation count
void increment_file_operations(MetricsCollector* collector) {
    if (collector == NULL) return;
    collector->file_operations_count++;
    collector->total_operations_count++;
    printf("Incremented file operations counter\n");
}

// Report collected metrics
void report_metrics(const MetricsCollector* collector) {
    if (collector == NULL) return;
    time_t current_time = time(NULL);
    double elapsed_seconds = difftime(current_time, collector->start_time);
    
    printf("\n===== System Metrics Report =====\n");
    printf("%-30s %.0f seconds\n", "Uptime:", elapsed_seconds);
    printf("%-30s %d\n", "Total Operations Recorded:", collector->total_operations_count);
    printf("---------------------------------\n");
    printf("%-30s %.2f ms\n", "Scheduling Latency (Total):", collector->scheduling_latency_total);
    printf("%-30s %.2f ms\n", "Memory Allocation Time (Total):", collector->memory_allocation_time_total);
    printf("%-30s %d\n", "File Operations Count:", collector->file_operations_count);
    
    // Calculate and print averages if operations occurred
    int non_file_ops = collector->total_operations_count - collector->file_operations_count;
    if (non_file_ops > 0) { // Avoid division by zero for averages
        printf("\n--- Average Metrics ---\n");
        // Note: These averages might be skewed if operations weren't balanced
        double avg_scheduling = (non_file_ops > 0) ? (collector->scheduling_latency_total / non_file_ops) : 0.0;
        double avg_memory = (non_file_ops > 0) ? (collector->memory_allocation_time_total / non_file_ops) : 0.0;
        printf("%-30s %.2f ms\n", "Average Scheduling Latency:", avg_scheduling); // Averaged over non-file ops
        printf("%-30s %.2f ms\n", "Average Memory Allocation Time:", avg_memory); // Averaged over non-file ops
    }
    
    printf("=================================\n");
}

int main() {
    printf("OS Metrics Collector Demo\n");
    printf("========================\n\n");
    
    // Declare the collector on the stack
    MetricsCollector collector;
    // Pass the address of the collector to functions
    initialize_metrics(&collector);
    
    // Simulate some operations
    printf("\n--- Simulating System Operations ---\n");
    
    // Simulate scheduling latency measurements
    record_scheduling_latency(&collector, 2.5);
    record_scheduling_latency(&collector, 1.8);
    record_scheduling_latency(&collector, 3.2);
    
    // Simulate memory allocation timings
    record_memory_allocation(&collector, 0.8);
    record_memory_allocation(&collector, 1.2);
    record_memory_allocation(&collector, 0.9);
    
    // Simulate file operations
    for (int i = 0; i < 5; i++) {
        increment_file_operations(&collector);
    }
    
    // Wait a moment to show some elapsed time
    printf("\nWaiting for 2 seconds...\n");
    sleep(2);
    
    // Generate report
    report_metrics(&collector);
    
    printf("\nMetrics collector demo finished.\n");
    return 0;
} 