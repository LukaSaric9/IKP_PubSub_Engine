#include "buffer.h"

// Initialize the dynamic buffer
void initializeBuffer(DynamicBuffer* buffer) {
    buffer->buffer = (TopicMessagePair*)malloc(INITIAL_CAPACITY * sizeof(TopicMessagePair));
    if (!buffer->buffer) {
        fprintf(stderr, "Failed to allocate memory for the buffer.\n");
        exit(EXIT_FAILURE);
    }
    buffer->size = 0;
    buffer->capacity = INITIAL_CAPACITY;
}

void storeTopicMessage(DynamicBuffer* buffer, const char* topic, const char* message) {
    printf("Storing topic to buffer: '%s', message: '%s'\n", topic, message);

    // Resize the buffer if needed
    if (buffer->size >= buffer->capacity) {
        size_t newCapacity = buffer->capacity * 2;
        if (newCapacity == 0) {
            newCapacity = 1; // Start with capacity 1 if buffer is empty
        }

        printf("Resizing buffer: old capacity = %zu, new capacity = %zu\n", buffer->capacity, newCapacity);

        TopicMessagePair* newBuffer = (TopicMessagePair*)realloc(buffer->buffer, newCapacity * sizeof(TopicMessagePair));
        if (!newBuffer) {
            fprintf(stderr, "Failed to reallocate memory for the buffer.\n");
            exit(EXIT_FAILURE);
        }

        buffer->buffer = newBuffer;
        buffer->capacity = newCapacity;
    }

    // Duplicate the topic string
    buffer->buffer[buffer->size].topic = _strdup(topic);
    if (!buffer->buffer[buffer->size].topic) {
        fprintf(stderr, "Failed to allocate memory for topic.\n");
        exit(EXIT_FAILURE);
    }

    // Duplicate the message string
    buffer->buffer[buffer->size].message = _strdup(message);
    if (!buffer->buffer[buffer->size].message) {
        fprintf(stderr, "Failed to allocate memory for message.\n");
        exit(EXIT_FAILURE);
    }

    // Increment buffer size
    buffer->size++;

    printf("Buffer updated: size = %zu, capacity = %zu\n", buffer->size, buffer->capacity);
}


// Free the dynamic buffer and its contents
void freeBuffer(DynamicBuffer* buffer) {
    for (size_t i = 0; i < buffer->size; i++) {
        free(buffer->buffer[i].topic);   // Free the topic string
        free(buffer->buffer[i].message); // Free the message string
    }
    free(buffer->buffer); // Free the buffer array
    buffer->buffer = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
}

// Print buffer contents (for debugging)
void printBufferContents(DynamicBuffer* buffer) {
    printf("Buffer Contents (Size: %zu, Capacity: %zu):\n", buffer->size, buffer->capacity);
    for (size_t i = 0; i < buffer->size; i++) {
        printf("  [%zu] Topic: %s, Message: %s\n", i, buffer->buffer[i].topic, buffer->buffer[i].message);
    }
}
