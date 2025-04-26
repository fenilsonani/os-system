CC = gcc
CFLAGS = -Wall -Wextra -pthread

TARGETS = scheduler memory_manager file_system lru_page_replacement metrics_collector

all: $(TARGETS)

scheduler: scheduler.c
	$(CC) $(CFLAGS) -o $@ $<

memory_manager: memory_manager.c
	$(CC) $(CFLAGS) -o $@ $<

file_system: file_system.c
	$(CC) $(CFLAGS) -o $@ $<

lru_page_replacement: lru_page_replacement.c
	$(CC) $(CFLAGS) -o $@ $<

metrics_collector: metrics_collector.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean 