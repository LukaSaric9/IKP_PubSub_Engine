#include "buffer.h"
/*
CircularBuffer* circular_buffer_init(size_t initial_capacity) {
    CircularBuffer* cb = (CircularBuffer*)malloc(sizeof(CircularBuffer));
    CHECK_ALLOC(cb);
    cb->buffer = (PublishTask*)malloc(initial_capacity * sizeof(PublishTask));
    CHECK_ALLOC(cb->buffer);
    cb->head = 0;
    cb->tail = 0;
    cb->capacity = initial_capacity;
    MUTEX_INIT(cb->mutex); // A mutex (cb->mutex) protects access to the buffer.
    CONDVAR_INIT(cb->not_empty);//Condition variables (not_empty and not_full) are used to signal producers and consumers. Producers add tasks and signal not_empty. Consumers wait on not_empty when the buffer is empty
    CONDVAR_INIT(cb->not_full); // 
    return cb;
}

void circular_buffer_add(CircularBuffer* cb, PublishTask task) {
    MUTEX_LOCK(cb->mutex);
    // Check if buffer is full
    size_t next_head = (cb->head + 1) % cb->capacity;
    if (next_head == cb->tail) {
        // Buffer is full; expand capacity
        size_t new_capacity = cb->capacity * 2;
        PublishTask* new_buffer = (PublishTask*)realloc(cb->buffer, new_capacity * sizeof(PublishTask)); //When the buffer is full, its capacity is doubled using realloc.
        CHECK_ALLOC(new_buffer);
        // If head is before tail in the buffer, need to rearrange
        if (cb->head < cb->tail) {
            memmove(&new_buffer[cb->capacity], &new_buffer[0], cb->head * sizeof(PublishTask));
            cb->head += cb->capacity;
        }
        cb->buffer = new_buffer;
        cb->capacity = new_capacity;
    }

    cb->buffer[cb->head] = task;
    cb->head = (cb->head + 1) % cb->capacity;

    CONDVAR_SIGNAL(cb->not_empty);
    MUTEX_UNLOCK(cb->mutex);
}

PublishTask circular_buffer_remove(CircularBuffer* cb) {
    MUTEX_LOCK(cb->mutex);
    while (cb->head == cb->tail) {
        // Buffer is empty; wait for not_empty signal
        CONDVAR_WAIT(cb->not_empty, cb->mutex);
    }

    PublishTask task = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % cb->capacity;

    // Optionally signal not_full if implementing bounded buffer
    // CONDVAR_SIGNAL(cb->not_full);

    MUTEX_UNLOCK(cb->mutex);
    return task;
}

void circular_buffer_destroy(CircularBuffer* cb) {
    free(cb->buffer);
    MUTEX_DESTROY(cb->mutex);
    // No need to destroy condition variables explicitly on Windows
    free(cb);
}*/
