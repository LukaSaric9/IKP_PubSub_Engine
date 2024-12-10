#include "buffer.h"

void initializeCircularBuffer(CircularBuffer* buffer) {
    buffer->buffer = (TopicMessagePair*)malloc(INITIAL_CAPACITY * sizeof(TopicMessagePair));
    if (!buffer->buffer) {
        fprintf(stderr, "Failed to allocate memory for the buffer.\n");
        exit(EXIT_FAILURE);
    }
    buffer->capacity = INITIAL_CAPACITY;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;  // Initialize size to 0
}

void storeTopicMessageCircular(CircularBuffer* buffer, const char* topic, const char* message) {
    // Allocate and copy topic and message
    char* topicCopy = _strdup(topic);
    char* messageCopy = _strdup(message);
    if (!topicCopy || !messageCopy) {
        printf("Failed to allocate memory for topic or message.\n");
        free(topicCopy);   // Ensure previously allocated memory is freed
        free(messageCopy);
        return;
    }

    // Check if the buffer is full and needs to grow
    if (buffer->size == buffer->capacity) {
        // Double the capacity, but do not exceed MAX_CAPACITY
        if (buffer->capacity < MAX_CAPACITY) {
            size_t newCapacity = buffer->capacity * 2;
            if (newCapacity > MAX_CAPACITY) {
                newCapacity = MAX_CAPACITY;
            }

            // Reallocate memory for the buffer
            TopicMessagePair* newBuffer = (TopicMessagePair*)malloc(newCapacity * sizeof(TopicMessagePair));
            if (!newBuffer) {
                printf("Failed to reallocate memory for the buffer.\n");
                free(topicCopy);
                free(messageCopy);
                return;
            }

            // Copy the existing messages to the new buffer
            size_t i = 0;
            size_t index = buffer->tail;
            while (index != buffer->head) {
                newBuffer[i].topic = buffer->buffer[index].topic;
                newBuffer[i].message = buffer->buffer[index].message;
                index = (index + 1) % buffer->capacity;
                i++;
            }

            // Free the old buffer and update the buffer pointer and capacity
            free(buffer->buffer);
            buffer->buffer = newBuffer;
            buffer->capacity = newCapacity;
            buffer->head = i;  // The new head will be the next empty position
            buffer->tail = 0;  // Reset tail to the beginning of the new buffer
        }
        else {
            // If capacity has reached MAX_CAPACITY, overwrite the oldest message
            free(buffer->buffer[buffer->tail].topic);
            free(buffer->buffer[buffer->tail].message);
            buffer->tail = (buffer->tail + 1) % buffer->capacity;
        }
    }

    // Store the new topic-message pair in the buffer
    buffer->buffer[buffer->head].topic = topicCopy;
    buffer->buffer[buffer->head].message = messageCopy;

    // Move the head forward (circularly)
    buffer->head = (buffer->head + 1) % buffer->capacity;

    // Increase size, unless we are overwriting (in case of circular buffer)
    if (buffer->size < buffer->capacity) {
        buffer->size++;
    }
}

TopicMessagePair readFromCircularBuffer(CircularBuffer* buffer) {
    if (buffer->size == 0) {
        TopicMessagePair empty = { NULL, NULL }; // Create an empty TopicMessagePair
        return empty; // Return the empty pair
    }

    // Get the message at the tail index
    TopicMessagePair message = buffer->buffer[buffer->tail];

    // Move the tail index forward (circularly)
    buffer->tail = (buffer->tail + 1) % buffer->capacity;

    // Decrease the size after reading
    buffer->size--;

    return message;
}

void freeCircularBuffer(CircularBuffer* buffer) {
    // Free the allocated memory for topic and message strings
    if (buffer->size > 0) {
        for (size_t i = 0; i < buffer->size; i++) {
            free(buffer->buffer[(buffer->tail + i) % buffer->capacity].topic);
            free(buffer->buffer[(buffer->tail + i) % buffer->capacity].message);
        }
    }
    free(buffer->buffer);
}

// Print the circular buffer contents (for debugging)
void printBufferContents(CircularBuffer* buffer) {
    printf("Buffer Contents (Capacity: %zu):\n", buffer->capacity);
    size_t index = buffer->tail;
    while (index != buffer->head) {
        printf("  Topic: %s, Message: %s\n", buffer->buffer[index].topic, buffer->buffer[index].message);
        index = (index + 1) % buffer->capacity;
    }
}