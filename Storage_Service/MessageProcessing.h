#pragma once
#ifndef MESSAGE_PROCESSING_H
#define MESSAGE_PROCESSING_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

#define BUFFER_SIZE 1024

typedef struct MessageRecord {
    char* topic;
    char* message;
    time_t timestamp;  // Store the time the message was received
    struct MessageRecord* next;  // For linked list implementation
} MessageRecord;

typedef struct { //What server sends
    char* topic;   // Topic string
    char* message; // Message string
} TopicMessagePair;

void SaveMessage(const char* topic, const char* message);
DWORD WINAPI ReceiveMessages(LPVOID lpParam);
void SearchMessages(const char* searchTopic);
void CleanupMessages();
void ReadAllMessages();

#endif // MESSAGE_PROCESsING_H