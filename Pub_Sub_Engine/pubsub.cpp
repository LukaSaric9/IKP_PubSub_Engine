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
    storeTopicMessageCircular(&publishedMessagesBuffer, topic, message);
    ReleaseMutex(publishedMessagesMutex);
    printf("Message stored in buffer for async processing: %s:%s\n", topic, message);

}

void PubSub_Cleanup() {
    CleanupGlobalData();
    printf("PubSub service cleaned up.\n");
}

// Process publisher messages
void ProcessPublisherMessage(SOCKET clientSocket, const char* message) {
    const char* delimiter = strchr(message, ':');
    if (delimiter != NULL) {
        size_t topicLength = delimiter - message;
        char topic[TOPIC_SIZE];
        char content[1024];

        if (topicLength >= sizeof(topic)) {
            printf("Topic too long, truncating.\n");
            topicLength = sizeof(topic) - 1;
        }
        
        strncpy_s(topic, message, topicLength);
        topic[topicLength] = '\0';
        
        strncpy_s(content, sizeof(content), delimiter + 1, _TRUNCATE);

        // Use the Publish interface
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

    while (true) {
        bytesReceived = recv(clientSocket, recvBuffer, BUFFER_LENGTH - 1, 0);
        if (bytesReceived > 0) {
            recvBuffer[bytesReceived] = '\0';

            if (strncmp(recvBuffer, "PUBLISHER:", 10) == 0) {
                ProcessPublisherMessage(clientSocket, recvBuffer + 10);
            }
            else if (strncmp(recvBuffer, "SUBSCRIBER:", 11) == 0) {
                ProcessSubscriberMessage(clientSocket, recvBuffer + 11);
            }
            else if (strncmp(recvBuffer, STORAGE_IDENTIFIER, strlen(STORAGE_IDENTIFIER)) == 0) {
                printf("Storage service connected.\n");
                storageSocket = clientSocket;
            }
            else {
                printf("Unknown client type. Message ignored.\n");
            }
        }
        else if (bytesReceived == 0) {
            printf("Client disconnected.\n");
            break;
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    closesocket(clientSocket);
}

// Notify all subscribers of a topic about a new message
void notifySubscribers(const char* topic, const char* message) {
    printf("Notifying subscribers for topic: '%s'\n", topic);
    char* client_message = format_for_client(topic, message);

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

// Send message to storage service
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

// Utility function to format TopicMessagePair as string
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

// Utility function to format message for client display
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
