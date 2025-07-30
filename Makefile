CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -pthread -std=c99
LDFLAGS = 

# macOS doesn't need -lrt, Linux does
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lrt
endif

# Original components
ORIGINAL_TARGETS = scheduler_original memory_manager_original file_system_original lru_page_replacement_original metrics_collector_original

# Enhanced components  
ENHANCED_TARGETS = scheduler memory_manager file_system_enhanced lru_enhanced metrics_enhanced

# All targets
TARGETS = $(ORIGINAL_TARGETS) $(ENHANCED_TARGETS)

all: $(TARGETS)
	@echo "All components built successfully!"
	@echo "Enhanced versions available: $(ENHANCED_TARGETS)"

# Enhanced versions (optimized implementations)
scheduler: scheduler.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

memory_manager: memory_manager.c  
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

file_system_enhanced: file_system_enhanced.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

lru_enhanced: lru_enhanced.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

metrics_enhanced: metrics_collector.c
	$(CC) $(CFLAGS) -DENHANCED -o $@ $< $(LDFLAGS)

# Original versions (for comparison)
scheduler_original: scheduler_original.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

memory_manager_original: memory_manager_original.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

file_system_original: file_system_original.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

lru_page_replacement_original: lru_page_replacement_original.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

metrics_collector_original: metrics_collector_original.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build only enhanced versions
enhanced: $(ENHANCED_TARGETS)
	@echo "Enhanced components built!"

# Build only original versions  
original: $(ORIGINAL_TARGETS)
	@echo "Original components built!"

# Performance test
test: $(ENHANCED_TARGETS)
	@echo "Running performance tests..."
	@echo "=== Scheduler Test ==="
	./scheduler
	@echo "\n=== Memory Manager Test ==="
	./memory_manager
	@echo "\n=== File System Test ==="
	./file_system_enhanced
	@echo "\n=== LRU Test ==="
	./lru_enhanced

# Clean up
clean:
	rm -f $(TARGETS)
	@echo "Cleaned up all binaries"

# Install (copy to /usr/local/bin)
install: $(ENHANCED_TARGETS)
	sudo cp $(ENHANCED_TARGETS) /usr/local/bin/
	@echo "Enhanced OS components installed to /usr/local/bin"

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/scheduler /usr/local/bin/memory_manager /usr/local/bin/file_system_enhanced /usr/local/bin/lru_enhanced /usr/local/bin/metrics_enhanced
	@echo "OS components uninstalled"

.PHONY: all enhanced original test clean install uninstall 