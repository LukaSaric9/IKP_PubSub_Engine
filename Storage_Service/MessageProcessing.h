#pragma once
#ifndef MESSAGE_PROCESSING_H
#define MESSAGE_PROCESSING_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

#define BUFFER_SIZE 1024

// Priority definitions
#define PRIORITY_HIGH 0
#define PRIORITY_MEDIUM 1
#define PRIORITY_LOW 2

typedef struct MessageRecord {
    char* topic;
    char* message;
    int priority;           // Add priority field
    time_t timestamp;       // Store the time the message was received
    struct MessageRecord* next;  // For linked list implementation
} MessageRecord;

typedef struct { //What server sends
    char* topic;   // Topic string
    char* message; // Message string
} TopicMessagePair;

void SaveMessage(const char* topic, const char* message, int priority);
DWORD WINAPI ReceiveMessages(LPVOID lpParam);
void SearchMessages(const char* searchTopic);
void CleanupMessages();
void ReadAllMessages();
const char* getPriorityName(int priority);
const char* getPriorityIcon(int priority);

#endif // MESSAGE_PROCESSING_H