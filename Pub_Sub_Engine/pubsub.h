#ifndef PUBSUB_H
#define PUBSUB_H

#include "common.h"
#include "hashmap.h"
#include "buffer.h"
#include "dashboard.h"

// Global data structures
extern HashMap topicSubscribers;
extern CircularBuffer publishedMessagesBuffer;
extern HANDLE publishedMessagesMutex;
extern SOCKET storageSocket;

// Core PubSub interface functions
void Connect();
void Subscribe(const char* topic, SOCKET clientSocket);
void Publish(const char* topic, const char* message);
void PublishWithPriority(const char* topic, const char* message, int priority);
void PubSub_Cleanup();

// Message processing functions
void ProcessPublisherMessage(SOCKET clientSocket, const char* message);
void ProcessSubscriberMessage(SOCKET clientSocket, const char* message);
void ProcessClientMessage(SOCKET clientSocket);

// Notification and storage functions
void notifySubscribers(const char* topic, const char* message);
void notifySubscribersWithPriority(const char* topic, const char* message, int priority);
void SendToStorage(const char* message);

// Utility functions
char* format_struct_to_string(const TopicMessagePair* my_struct);
char* format_priority_struct_to_string(const PriorityTopicMessagePair* my_struct);
char* format_for_client(const char* topic, const char* message);
char* format_for_client_with_priority(const char* topic, const char* message, int priority);
const char* getPriorityName(int priority);

// Global data management
void InitializeGlobalData();
void CleanupGlobalData();

#endif // PUBSUB_H

