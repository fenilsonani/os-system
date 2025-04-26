# Advanced Operating System Components

This project implements key OS components as described in the article "Advanced Operating System Components: Implementing Scheduler, Memory Manager, and File System." The implementations are simplified examples intended for educational purposes.

## Components

1. **Scheduler**: A real-time process scheduler using priority queues
2. **Memory Manager**: A page-based memory allocation system
3. **File System**: A basic in-memory file system
4. **LRU Page Replacement**: Implementation of Least Recently Used page replacement algorithm
5. **Metrics Collector**: System for collecting and reporting OS performance metrics

## Requirements

- GCC compiler
- POSIX-compliant system (Linux/macOS)
- pthread library

## Building the Project

To build all components, run:

```bash
make
```

This will compile all five components into separate executables.

To clean up the build files:

```bash
make clean
```

## Running the Components

Each component can be run independently:

```bash
# Run the scheduler demo
./scheduler

# Run the memory manager demo
./memory_manager

# Run the file system demo
./file_system

# Run the LRU page replacement algorithm demo
./lru_page_replacement

# Run the metrics collector demo
./metrics_collector
```

## Component Descriptions

### Scheduler

Demonstrates a process scheduler that manages processes based on priorities and burst times. It uses a priority queue and pthread for concurrent execution.

### Memory Manager

Implements a simple memory manager with page allocation and deallocation. It manages a fixed pool of memory pages and handles memory request operations.

### File System

A basic in-memory file system that supports file creation, reading, and deletion. It maintains file metadata and provides a directory listing capability.

### LRU Page Replacement

Implements the Least Recently Used algorithm for page replacement. This component demonstrates how memory pages are managed when memory is full and new pages need to be loaded.

### Metrics Collector

A system for collecting and reporting performance metrics related to scheduling latency, memory allocation times, and file operations.

## Contributing

This is an educational project based on simplified implementations. Feel free to enhance any component or add new features.

## License

[MIT License](LICENSE) 