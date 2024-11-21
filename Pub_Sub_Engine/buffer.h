#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "common.h"
/*
// Define the structure for a publish task
typedef struct PublishTask {
    char topic[MAX_TOPIC_LENGTH];
    char message[MAX_MESSAGE_LENGTH];
} PublishTask;

// Circular buffer structure
typedef struct CircularBuffer {
    PublishTask* buffer;
    size_t head;
    size_t tail;
    size_t capacity;
    Mutex mutex;
    CondVar not_empty;
    CondVar not_full;
} CircularBuffer;

// Initialize the circular buffer
CircularBuffer* circular_buffer_init(size_t initial_capacity);

// Add a task to the buffer
void circular_buffer_add(CircularBuffer* cb, PublishTask task);

// Remove a task from the buffer
PublishTask circular_buffer_remove(CircularBuffer* cb);

// Destroy the circular buffer
void circular_buffer_destroy(CircularBuffer* cb);
*/
#endif // CIRCULAR_BUFFER_H

