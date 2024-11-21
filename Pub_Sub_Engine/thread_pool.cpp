#include "thread_pool.h"
/*
static DWORD WINAPI thread_worker(LPVOID arg) { //Each worker thread continuously fetches tasks from the queue and executes them. They wait on a condition variable when the task queue is empty.
    ThreadPool* pool = (ThreadPool*)arg;
    while (1) {
        MUTEX_LOCK(pool->mutex); //A mutex (pool->mutex) protects access to the task queue.
        while (pool->task_head == pool->task_tail && !pool->shutdown) {
            CONDVAR_WAIT(pool->condition, pool->mutex); //Condition variables (pool->condition) are used to signal worker threads when new tasks are available.
        }

        if (pool->shutdown) {
            MUTEX_UNLOCK(pool->mutex);
            break;
        }

        // Get the next task
        Task task = pool->tasks[pool->task_tail];
        pool->task_tail = (pool->task_tail + 1) % pool->task_queue_capacity;
        MUTEX_UNLOCK(pool->mutex);

        // Execute the task
        task.function(task.arg);
    }
    return 0;
}

ThreadPool* thread_pool_init(size_t num_threads) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    CHECK_ALLOC(pool);
    pool->thread_count = num_threads;
    pool->threads = (HANDLE*)malloc(num_threads * sizeof(HANDLE));
    CHECK_ALLOC(pool->threads);
    pool->task_queue_capacity = 1024; // Initial capacity
    pool->tasks = (Task*)malloc(pool->task_queue_capacity * sizeof(Task));
    CHECK_ALLOC(pool->tasks);
    pool->task_queue_size = 0;
    pool->task_head = 0;
    pool->task_tail = 0;
    MUTEX_INIT(pool->mutex);
    CONDVAR_INIT(pool->condition);
    pool->shutdown = false;

    // Create worker threads
    for (size_t i = 0; i < num_threads; ++i) {
        pool->threads[i] = CreateThread(
            NULL,
            0,
            thread_worker,
            pool,
            0,
            NULL
        );
        if (pool->threads[i] == NULL) {
            HANDLE_ERROR("CreateThread failed");
        }
    }

    return pool;
}

void thread_pool_add_task(ThreadPool* pool, void (*function)(void*), void* arg) {
    MUTEX_LOCK(pool->mutex);

    // Check if task queue is full
    size_t next_head = (pool->task_head + 1) % pool->task_queue_capacity;
    if (next_head == pool->task_tail) {
        // Need to expand the task queue
        size_t new_capacity = pool->task_queue_capacity * 2;
        Task* new_tasks = (Task*)malloc(new_capacity * sizeof(Task));
        CHECK_ALLOC(new_tasks);
        size_t i = pool->task_tail;
        size_t j = 0;
        while (i != pool->task_head) {
            new_tasks[j++] = pool->tasks[i];
            i = (i + 1) % pool->task_queue_capacity;
        }
        free(pool->tasks);
        pool->tasks = new_tasks;
        pool->task_tail = 0;
        pool->task_head = j;
        pool->task_queue_capacity = new_capacity;
    }

    // Add the task
    pool->tasks[pool->task_head].function = function;
    pool->tasks[pool->task_head].arg = arg;
    pool->task_head = (pool->task_head + 1) % pool->task_queue_capacity;
    pool->task_queue_size++;

    // Signal a worker thread
    CONDVAR_SIGNAL(pool->condition);
    MUTEX_UNLOCK(pool->mutex);
}

void thread_pool_destroy(ThreadPool* pool) {
    // Signal all threads to shutdown
    MUTEX_LOCK(pool->mutex);
    pool->shutdown = true;
    CONDVAR_BROADCAST(pool->condition);
    MUTEX_UNLOCK(pool->mutex);

    // Wait for all threads to finish
    WaitForMultipleObjects(
        (DWORD)pool->thread_count,
        pool->threads,
        TRUE,
        INFINITE
    );

    // Close thread handles
    for (size_t i = 0; i < pool->thread_count; ++i) {
        CloseHandle(pool->threads[i]);
    }

    free(pool->threads);
    free(pool->tasks);
    MUTEX_DESTROY(pool->mutex);
    free(pool);
}

*/