#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"

// Define a structure to represent a topic-message pair
typedef struct {
    char* topic;   // Topic string
    char* message; // Message string
} TopicMessagePair;

// Define the dynamic buffer structure
typedef struct {
    TopicMessagePair* buffer; // Array of topic-message pairs
    size_t size;              // Current number of elements
    size_t capacity;          // Current capacity of the buffer
} DynamicBuffer;

// Function declarations
void initializeBuffer(DynamicBuffer* buffer);
void storeTopicMessage(DynamicBuffer* buffer, const char* topic, const char* message);
void freeBuffer(DynamicBuffer* buffer);
void printBufferContents(DynamicBuffer* buffer);

#endif // BUFFER_H
