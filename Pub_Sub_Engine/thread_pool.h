#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"
/*
// Define the structure for a task
typedef struct Task {
    void (*function)(void* arg);
    void* arg;
} Task;

// Thread pool structure
typedef struct ThreadPool { // Contains an array of thread handles, a task queue, and synchronization primitives
    HANDLE* threads;
    size_t thread_count;
    Task* tasks;
    size_t task_queue_size;
    size_t task_queue_capacity;
    size_t task_head;
    size_t task_tail;
    Mutex mutex;
    CondVar condition;
    bool shutdown;
} ThreadPool;

// Initialize the thread pool
ThreadPool* thread_pool_init(size_t num_threads);

// Add a task to the thread pool
void thread_pool_add_task(ThreadPool* pool, void (*function)(void*), void* arg);

// Destroy the thread pool
void thread_pool_destroy(ThreadPool* pool);
*/
#endif // THREAD_POOL_H

