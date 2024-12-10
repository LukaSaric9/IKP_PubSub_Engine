#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"

// Define a structure to represent a topic-message pair
typedef struct {
    char* topic;   // Topic string
    char* message; // Message string
} TopicMessagePair;

// Define the circular buffer structure
typedef struct {
    TopicMessagePair* buffer;  // Array of topic-message pairs
    size_t capacity;           // Capacity of the buffer
    size_t head;               // Index for the next element to be written
    size_t tail;               // Index for the next element to be read
    size_t size;                // Current number of messages in the buffer
} CircularBuffer;

// Function declarations
void initializeCircularBuffer(CircularBuffer* buffer);
void storeTopicMessageCircular(CircularBuffer* buffer, const char* topic, const char* message);
TopicMessagePair readFromCircularBuffer(CircularBuffer* buffer);
void freeCircularBuffer(CircularBuffer* buffer);
void printBufferContents(CircularBuffer* buffer);

#endif // BUFFER_H
