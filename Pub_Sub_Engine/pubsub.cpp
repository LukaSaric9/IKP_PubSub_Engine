#define _CRT_SECURE_NO_WARNINGS
#include "pubsub.h"

// Global data structures - moved from thread_pool.cpp
HashMap topicSubscribers;
CircularBuffer publishedMessagesBuffer;
HANDLE publishedMessagesMutex;
SOCKET storageSocket = INVALID_SOCKET;

// Initialize global structures and mutex
void InitializeGlobalData() {
    initializeHashMapWithMutex(&topicSubscribers);
    initializeCircularBuffer(&publishedMessagesBuffer);
    publishedMessagesMutex = CreateMutex(NULL, FALSE, NULL);
}

// Clean up global structures
void CleanupGlobalData() {
    freeHashMapWithMutex(&topicSubscribers);
    freeCircularBuffer(&publishedMessagesBuffer);
    CloseHandle(publishedMessagesMutex);
}

// Core PubSub interface implementation
void Connect() {
    InitializeGlobalData();
    printf("PubSub service initialized.\n");
}

void Subscribe(const char* topic, SOCKET clientSocket) {
    insertIntoHashMapWithLock(&topicSubscribers, topic, clientSocket);
    printf("Client subscribed to topic: %s\n", topic);
}

void Publish(const char* topic, const char* message) {
    WaitForSingleObject(publishedMessagesMutex, INFINITE);
    // Default to MEDIUM priority for backward compatibility
    storePriorityTopicMessageCircular(&publishedMessagesBuffer, topic, message, PRIORITY_MEDIUM);
    ReleaseMutex(publishedMessagesMutex);
    printf("Message stored in buffer for async processing: %s:%s\n", topic, message);
}

void PublishWithPriority(const char* topic, const char* message, int priority) {
    WaitForSingleObject(publishedMessagesMutex, INFINITE);
    storePriorityTopicMessageCircular(&publishedMessagesBuffer, topic, message, priority);
    ReleaseMutex(publishedMessagesMutex);
    printf("Message stored in buffer for async processing: %s:%s (Priority: %s)\n", 
           topic, message, getPriorityName(priority));
}

void PubSub_Cleanup() {
    CleanupGlobalData();
    printf("PubSub service cleaned up.\n");
}

// Process publisher messages with priority support
void ProcessPublisherMessage(SOCKET clientSocket, const char* message) {
    // Try to parse new format: PRIORITY:topic:message
    const char* firstDelimiter = strchr(message, ':');
    if (firstDelimiter != NULL) {
        // Check if first part is a number (priority)
        char priorityStr[10];
        size_t priorityLength = firstDelimiter - message;
        
        if (priorityLength < sizeof(priorityStr)) {
            strncpy_s(priorityStr, message, priorityLength);
            priorityStr[priorityLength] = '\0';
            
            // Try to parse as priority
            char* endPtr;
            long priority = strtol(priorityStr, &endPtr, 10);
            
            if (*endPtr == '\0' && priority >= PRIORITY_HIGH && priority <= PRIORITY_LOW) {
                // New format: PRIORITY:topic:message
                const char* secondDelimiter = strchr(firstDelimiter + 1, ':');
                if (secondDelimiter != NULL) {
                    // Extract topic
                    size_t topicLength = secondDelimiter - (firstDelimiter + 1);
                    char topic[TOPIC_SIZE];
                    char content[1024];

                    if (topicLength >= sizeof(topic)) {
                        printf("Topic too long, truncating.\n");
                        topicLength = sizeof(topic) - 1;
                    }
                    
                    strncpy_s(topic, firstDelimiter + 1, topicLength);
                    topic[topicLength] = '\0';
                    
                    strncpy_s(content, sizeof(content), secondDelimiter + 1, _TRUNCATE);

                    // Use the priority-aware Publish interface
                    PublishWithPriority(topic, content, (int)priority);
                    updateMessageStats(topic, content, priority);
                    return;
                }
            }
        }
        
        // Fall back to old format: topic:message
        size_t topicLength = firstDelimiter - message;
        char topic[TOPIC_SIZE];
        char content[1024];

        if (topicLength >= sizeof(topic)) {
            printf("Topic too long, truncating.\n");
            topicLength = sizeof(topic) - 1;
        }
        
        strncpy_s(topic, message, topicLength);
        topic[topicLength] = '\0';
        
        strncpy_s(content, sizeof(content), firstDelimiter + 1, _TRUNCATE);

        // Use the standard Publish interface (defaults to MEDIUM priority)
        Publish(topic, content);
    } 
    else {
        printf("Invalid publisher message format.\n");
    }
}

// Process subscriber messages
void ProcessSubscriberMessage(SOCKET clientSocket, const char* message) {
    char topic[TOPIC_SIZE];
    size_t messageLength = strlen(message);

    if (messageLength >= TOPIC_SIZE) {
        printf("Topic too long, truncating...\n");
        messageLength = TOPIC_SIZE - 1;
    }

    strncpy_s(topic, message, messageLength);
    topic[messageLength] = '\0';
    
    // Use the Subscribe interface
    Subscribe(topic, clientSocket);

    // Debug: Print current HashMap contents
    printf("Current HashMap Contents:\n");
    WaitForSingleObject(hashMapMutex, INFINITE);
    
    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        HashMapNode* currentNode = topicSubscribers.buckets[i];
        if (currentNode) {
            printf("Bucket %d:\n", i);
        }

        while (currentNode) {
            printf("  Topic: %s\n", currentNode->topic);
            SubscriberNode* subscriber = currentNode->subscribers;
            while (subscriber) {
                printf("    Subscriber Socket: %lld\n", (long long)subscriber->socket);
                subscriber = subscriber->next;
            }
            currentNode = currentNode->next;
        }
    }
    
    ReleaseMutex(hashMapMutex);
}

// Process general client messages 
void ProcessClientMessage(SOCKET clientSocket) {
    char recvBuffer[BUFFER_LENGTH];
    int bytesReceived;
    bool clientTypeIdentified = false;
    char clientType[20] = "";

    while (true) {
        bytesReceived = recv(clientSocket, recvBuffer, BUFFER_LENGTH - 1, 0);
        if (bytesReceived > 0) {
            recvBuffer[bytesReceived] = '\0';

            if (strncmp(recvBuffer, "PUBLISHER:", 10) == 0) {
                if (!clientTypeIdentified) {
                    strcpy_s(clientType, sizeof(clientType), "PUBLISHER");
                    updateConnectionStats("PUBLISHER", 1);
                    clientTypeIdentified = true;
                }
                ProcessPublisherMessage(clientSocket, recvBuffer + 10);
            }
            else if (strncmp(recvBuffer, "SUBSCRIBER:", 11) == 0) {
                if (!clientTypeIdentified) {
                    strcpy_s(clientType, sizeof(clientType), "SUBSCRIBER");
                    updateConnectionStats("SUBSCRIBER", 1); 
                    clientTypeIdentified = true;
                }
                ProcessSubscriberMessage(clientSocket, recvBuffer + 11);
            }
            else if (strncmp(recvBuffer, STORAGE_IDENTIFIER, strlen(STORAGE_IDENTIFIER)) == 0) {
                printf("Storage service connected.\n");
                storageSocket = clientSocket;
                if (!clientTypeIdentified) {
                    strcpy_s(clientType, sizeof(clientType), "STORAGE");
                    updateConnectionStats("STORAGE", 1);
                    clientTypeIdentified = true;
                }
            }
            else {
                printf("Unknown client type. Message ignored.\n");
            }
        }
        else if (bytesReceived == 0) {
            printf("Client disconnected.\n");
            // Track disconnection
            if (clientTypeIdentified) {
                updateConnectionStats(clientType, -1);
            }
            break;
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            // Track disconnection on error
            if (clientTypeIdentified) {
                updateConnectionStats(clientType, -1);
            }
            break;
        }
    }

    closesocket(clientSocket);
}

// New function to notify subscribers WITH priority information
void notifySubscribersWithPriority(const char* topic, const char* message, int priority) {
    printf("Notifying subscribers for topic: '%s' with %s priority\n", topic, getPriorityName(priority));
    char* client_message = format_for_client_with_priority(topic, message, priority);

    SubscriberNode* subscribers = getSubscribersWithLock(&topicSubscribers, topic);

    while (subscribers) {
        SOCKET subscriberSocket = subscribers->socket;

        int sendResult = send(subscriberSocket, client_message, strlen(client_message), 0);
        if (sendResult == SOCKET_ERROR) {
            printf("Failed to send message to subscriber. Error: %d\n", WSAGetLastError());
        }
        else {
            printf("Message sent to subscriber (socket: %lld): '%s'\n", (long long)subscriberSocket, client_message);
        }

        subscribers = subscribers->next;
    }
    
    free(client_message);
    printf("Finished notifying subscribers for topic: '%s'\n", topic);
}

// Keep the old function for backward compatibility
void notifySubscribers(const char* topic, const char* message) {
    notifySubscribersWithPriority(topic, message, PRIORITY_MEDIUM);
}

// Send message to storage service WITH priority information
void SendToStorage(const char* message) {
    if (storageSocket != INVALID_SOCKET) {
        int bytesSent = send(storageSocket, message, (int)strlen(message), 0);
        if (bytesSent == SOCKET_ERROR) {
            printf("Failed to send message to storage service. Error: %d\n", WSAGetLastError());
        }
        else {
            printf("Message sent to storage service: %s\n", message);
        }
    }
}

// Updated utility function to include priority in storage format
char* format_priority_struct_to_string(const PriorityTopicMessagePair* my_struct) {
    // Format as: PRIORITY:topic:message (so storage can parse priority)
    size_t length = strlen(my_struct->topic) + strlen(my_struct->message) + 20; // Extra space for priority
    char* formatted_string = (char*)malloc(length);
    
    if (!formatted_string) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Include priority in the message sent to storage
    sprintf(formatted_string, "%d:%s:%s", my_struct->priority, my_struct->topic, my_struct->message);
    return formatted_string;
}

// Utility function to format TopicMessagePair as string (backward compatibility)
char* format_struct_to_string(const TopicMessagePair* my_struct) {
    size_t length = strlen(my_struct->topic) + strlen(my_struct->message) + 2;
    char* formatted_string = (char*)malloc(length);
    
    if (!formatted_string) {
        perror("Failed to allocate memory");
        return NULL;
    }

    sprintf(formatted_string, "%s:%s", my_struct->topic, my_struct->message);
    return formatted_string;
}

// Enhanced function to format message for client display WITH priority
char* format_for_client_with_priority(const char* topic, const char* message, int priority) {
    // Format as: [PRIORITY]topic: message
    size_t length = strlen(topic) + strlen(message) + 20; // Extra space for priority
    char* formatted_string = (char*)malloc(length);
    
    if (!formatted_string) {
        perror("Failed to allocate memory");
        return NULL;
    }

    sprintf(formatted_string, "[%d]%s: %s", priority, topic, message);
    return formatted_string;
}

// Keep backward compatibility version
char* format_for_client(const char* topic, const char* message) {
    size_t length = strlen(topic) + strlen(message) + 3;
    char* formatted_string = (char*)malloc(length);
    
    if (!formatted_string) {
        perror("Failed to allocate memory");
        return NULL;
    }

    sprintf(formatted_string, "%s: %s", topic, message);
    return formatted_string;
}

