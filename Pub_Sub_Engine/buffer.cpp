#include "buffer.h"

void initializeCircularBuffer(CircularBuffer* buffer) {
    buffer->buffer = (PriorityTopicMessagePair*)malloc(INITIAL_CAPACITY * sizeof(PriorityTopicMessagePair));
    if (!buffer->buffer) {
        fprintf(stderr, "Failed to allocate memory for the buffer.\n");
        exit(EXIT_FAILURE);
    }
    buffer->capacity = INITIAL_CAPACITY;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;  // Initialize size to 0
}

// Backward compatibility function - defaults to MEDIUM priority
void storeTopicMessageCircular(CircularBuffer* buffer, const char* topic, const char* message) {
    storePriorityTopicMessageCircular(buffer, topic, message, PRIORITY_MEDIUM);
}

// Enhanced function that stores with priority
void storePriorityTopicMessageCircular(CircularBuffer* buffer, const char* topic, const char* message, int priority) {
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
            PriorityTopicMessagePair* newBuffer = (PriorityTopicMessagePair*)malloc(newCapacity * sizeof(PriorityTopicMessagePair));
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
                newBuffer[i] = buffer->buffer[index];
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

    // Store the new priority topic-message pair in the buffer
    buffer->buffer[buffer->head].topic = topicCopy;
    buffer->buffer[buffer->head].message = messageCopy;
    buffer->buffer[buffer->head].priority = priority;
    buffer->buffer[buffer->head].timestamp = time(NULL);

    // Move the head forward (circularly)
    buffer->head = (buffer->head + 1) % buffer->capacity;

    // Increase size, unless we are overwriting (in case of circular buffer)
    if (buffer->size < buffer->capacity) {
        buffer->size++;
    }
}

// Backward compatibility function
TopicMessagePair readFromCircularBuffer(CircularBuffer* buffer) {
    PriorityTopicMessagePair priorityPair = readPriorityFromCircularBuffer(buffer);
    TopicMessagePair result = { priorityPair.topic, priorityPair.message };
    return result;
}

// Read next message in FIFO order (standard circular buffer behavior)
PriorityTopicMessagePair readPriorityFromCircularBuffer(CircularBuffer* buffer) {
    if (buffer->size == 0) {
        PriorityTopicMessagePair empty = { NULL, NULL, PRIORITY_MEDIUM, 0 };
        return empty;
    }

    // Get the message at the tail index
    PriorityTopicMessagePair message = buffer->buffer[buffer->tail];

    // Move the tail index forward (circularly)
    buffer->tail = (buffer->tail + 1) % buffer->capacity;

    // Decrease the size after reading
    buffer->size--;

    return message;
}

// NEW: Read the highest priority message from buffer
PriorityTopicMessagePair readHighestPriorityFromCircularBuffer(CircularBuffer* buffer) {
    if (buffer->size == 0) {
        PriorityTopicMessagePair empty = { NULL, NULL, PRIORITY_MEDIUM, 0 };
        return empty;
    }

    // Find the highest priority message
    size_t bestIndex = buffer->tail;
    int highestPriority = PRIORITY_LOW + 1; // Start with lower than lowest priority
    size_t currentIndex = buffer->tail;
    
    // Search through all messages in the buffer
    for (size_t i = 0; i < buffer->size; i++) {
        if (buffer->buffer[currentIndex].priority < highestPriority) {
            highestPriority = buffer->buffer[currentIndex].priority;
            bestIndex = currentIndex;
        }
        currentIndex = (currentIndex + 1) % buffer->capacity;
    }

    // Get the highest priority message
    PriorityTopicMessagePair result = buffer->buffer[bestIndex];

    // Remove this message from the buffer by shifting elements
    if (bestIndex != buffer->tail) {
        // If it's not the tail, we need to shift elements
        size_t currentPos = bestIndex;
        while (currentPos != buffer->tail) {
            size_t prevPos = (currentPos - 1 + buffer->capacity) % buffer->capacity;
            buffer->buffer[currentPos] = buffer->buffer[prevPos];
            currentPos = prevPos;
        }
    }

    // Move tail forward
    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->size--;

    return result;
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
    printf("Buffer Contents (Capacity: %zu, Size: %zu):\n", buffer->capacity, buffer->size);
    size_t index = buffer->tail;
    for (size_t i = 0; i < buffer->size; i++) {
        printf("  Topic: %s, Message: %s, Priority: %s\n", 
               buffer->buffer[index].topic, 
               buffer->buffer[index].message,
               getPriorityName(buffer->buffer[index].priority));
        index = (index + 1) % buffer->capacity;
    }
}

// Utility function to get priority name
const char* getPriorityName(int priority) {
    switch (priority) {
        case PRIORITY_HIGH: return "HIGH";
        case PRIORITY_MEDIUM: return "MEDIUM";
        case PRIORITY_LOW: return "LOW";
        default: return "UNKNOWN";
    }
}