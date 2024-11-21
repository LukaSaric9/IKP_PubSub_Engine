#ifndef COMMON_H
#define COMMON_H
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <mutex>

// --- Common Macros ---
#define CHECK_ALLOC(ptr) \
    if ((ptr) == NULL) { \
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    }

#define LOG_INFO(msg, ...) \
    fprintf(stdout, "[INFO] (%s:%d) " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR(msg, ...) \
    fprintf(stderr, "[ERROR] (%s:%d) " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// --- Project-wide Constants ---
#define MAX_TOPIC_LENGTH 128    // Maximum length for a topic name
#define MAX_MESSAGE_LENGTH 512  // Maximum length for a message
#define INITIAL_BUFFER_SIZE 8   // Initial size for the circular buffer
#define HASHMAP_SIZE 128 

// --- Thread Safety Utilities ---
typedef CRITICAL_SECTION Mutex;

// Initialize a mutex
#define MUTEX_INIT(mutex) InitializeCriticalSection(&(mutex))

// Lock a mutex
#define MUTEX_LOCK(mutex) EnterCriticalSection(&(mutex))

// Unlock a mutex
#define MUTEX_UNLOCK(mutex) LeaveCriticalSection(&(mutex))

// Destroy a mutex
#define MUTEX_DESTROY(mutex) DeleteCriticalSection(&(mutex))

// --- Condition Variable Utilities ---
typedef CONDITION_VARIABLE CondVar;

// Initialize a condition variable
#define CONDVAR_INIT(cond) InitializeConditionVariable(&(cond))

// Wait on a condition variable
#define CONDVAR_WAIT(cond, mutex) SleepConditionVariableCS(&(cond), &(mutex), INFINITE)

// Signal a condition variable
#define CONDVAR_SIGNAL(cond) WakeConditionVariable(&(cond))

// Broadcast a condition variable
#define CONDVAR_BROADCAST(cond) WakeAllConditionVariable(&(cond))

// --- Timestamp Utility ---
static inline char* get_timestamp() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    static char timestamp[20];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);
    return timestamp;
}

// --- Error Handling ---
#define HANDLE_ERROR(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

// Example utility function for safe string duplication.
static inline char* safe_strdup(const char* str) {
    char* dup = strdup(str);
    CHECK_ALLOC(dup);
    return dup;
}
*/
#endif // COMMON_H
