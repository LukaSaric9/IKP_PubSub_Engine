#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"

// Priority definitions
#define PRIORITY_HIGH 0
#define PRIORITY_MEDIUM 1
#define PRIORITY_LOW 2

// Define a structure to represent a priority topic-message pair
typedef struct {
    char* topic;      // Topic string
    char* message;    // Message string
    int priority;     // Message priority (0=HIGH, 1=MEDIUM, 2=LOW)
    time_t timestamp; // When the message was added
} PriorityTopicMessagePair;

// Keep the old structure for backward compatibility
typedef struct {
    char* topic;   // Topic string
    char* message; // Message string
} TopicMessagePair;

// Define the circular buffer structure
typedef struct {
    PriorityTopicMessagePair* buffer;  // Array of priority topic-message pairs
    size_t capacity;                   // Capacity of the buffer
    size_t head;                       // Index for the next element to be written
    size_t tail;                       // Index for the next element to be read
    size_t size;                       // Current number of messages in the buffer
} CircularBuffer;

// Enhanced function declarations
void initializeCircularBuffer(CircularBuffer* buffer);
void storeTopicMessageCircular(CircularBuffer* buffer, const char* topic, const char* message);
void storePriorityTopicMessageCircular(CircularBuffer* buffer, const char* topic, const char* message, int priority);
TopicMessagePair readFromCircularBuffer(CircularBuffer* buffer);
PriorityTopicMessagePair readPriorityFromCircularBuffer(CircularBuffer* buffer);
PriorityTopicMessagePair readHighestPriorityFromCircularBuffer(CircularBuffer* buffer);
void freeCircularBuffer(CircularBuffer* buffer);
void printBufferContents(CircularBuffer* buffer);

// Utility functions
const char* getPriorityName(int priority);

#endif // BUFFER_H