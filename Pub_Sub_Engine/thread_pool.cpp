#define _CRT_SECURE_NO_WARNINGS
#include "thread_pool.h"  
#include "hashmap.h"
#include "buffer.h"


// Thread pool and client queue definitions
HANDLE threadPool[MAX_THREADS];
HANDLE clientQueueMutex;
SOCKET clientQueue[MAX_THREADS];
int queueCount = 0;
SOCKET storageSocket = INVALID_SOCKET;


// Define the global data structures
HashMap topicSubscribers; // Use the custom HashMap for topic-subscriber mapping
CircularBuffer publishedMessagesBuffer;
HANDLE publishedMessagesMutex; // Mutex for publishedMessages
HANDLE messageThreadPool[MAX_MESSAGE_THREADS];

void notifySubscribers(const char* topic, const char* message);
void SendToStorage(const char* message);
char* format_struct_to_string(const TopicMessagePair* my_struct);
char* format_for_client(const char* topic, const char* message);

// Initialize global structures and mutex
void InitializeGlobalData() {
    initializeHashMapWithMutex(&topicSubscribers);
    initializeCircularBuffer(&publishedMessagesBuffer);
    publishedMessagesMutex = CreateMutex(NULL, FALSE, NULL); // Mutex for publishedMessages
}

// Clean up global structures
void CleanupGlobalData() {
    freeHashMapWithMutex(&topicSubscribers);
    freeCircularBuffer(&publishedMessagesBuffer);
    CloseHandle(publishedMessagesMutex);
}

// Process publisher messages
void ProcessPublisherMessage(SOCKET clientSocket, const char* message) {
    const char* delimeter = strchr(message, ':');
    if (delimeter != NULL) {
        size_t topicLength = delimeter - message;
        char topic[TOPIC_SIZE];
        char content[1024];

        if (topicLength >= sizeof(topic)) {
            printf("Topic too long, truncating.\n");
            topicLength = sizeof(topic) - 1;
        }
        strncpy_s(topic, message, topicLength);
        topic[topicLength] = '\0';

        strncpy_s(content, sizeof(content), delimeter + 1, _TRUNCATE);

        WaitForSingleObject(publishedMessagesMutex, INFINITE);;
        storeTopicMessageCircular(&publishedMessagesBuffer, topic, content); // Store the topic-message pair
        printBufferContents(&publishedMessagesBuffer);              // Optional: Debug output
        ReleaseMutex(publishedMessagesMutex);

    } 
    else {
        printf("Invalid publisher message format.\n");
    }

}

void notifySubscribers(const char* topic, const char* message) {
    printf("Notifying subscribers for topic: '%s'\n", topic);
    char* client_message = format_for_client(topic, message);

    // Lock the hash map to safely access subscribers
    SubscriberNode* subscribers = getSubscribersWithLock(&topicSubscribers, topic);

    while (subscribers) {
        SOCKET subscriberSocket = subscribers->socket;

        // Send the message to the subscriber
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
    
    // Add the subscriber to the topic in the hash map
    insertIntoHashMapWithLock(&topicSubscribers, topic, clientSocket);

    // Print the entire content of the hash map
    printf("Current HashMap Contents:\n");

    // Lock the hash map for safe access
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

    // Unlock the hash map
    ReleaseMutex(hashMapMutex);
}



// Process general client messages
void ProcessClientMessage(SOCKET clientSocket) {
    char recvBuffer[BUFFER_LENGTH];
    int bytesReceived;

    while (true) {
        bytesReceived = recv(clientSocket, recvBuffer, BUFFER_LENGTH - 1, 0);
        if (bytesReceived > 0) {
            recvBuffer[bytesReceived] = '\0'; // Null-terminate the message

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

// Send the message to the storage service
void SendToStorage(const char* message) {
    if (storageSocket != INVALID_SOCKET) {
        int bytesSent = send(storageSocket, message, (int)strlen(message), 0);
        if (bytesSent == SOCKET_ERROR) {
            printf("Failed to send message to storage service. Error: %d\n", WSAGetLastError());
        }
    }
}



// Worker function for threads
DWORD WINAPI WorkerFunction(LPVOID lpParam) {
    while (true) {
        SOCKET clientSocket = INVALID_SOCKET;

        // Wait for a client in the queue
        WaitForSingleObject(clientQueueMutex, INFINITE);
        if (queueCount > 0) {
            clientSocket = clientQueue[--queueCount];
        }
        ReleaseMutex(clientQueueMutex);

        if (clientSocket != INVALID_SOCKET) {
            ProcessClientMessage(clientSocket);
        }
        else {
            Sleep(10); // Prevent busy-waiting
        }
    }
    return 0;
}

DWORD WINAPI MessageWorkerFunction(LPVOID lpParam) {
    while (true) {
        WaitForSingleObject(publishedMessagesMutex, INFINITE);

        // Check if the buffer contains any messages
        if (publishedMessagesBuffer.size > 0) {
            // Read the first message from the circular buffer
            TopicMessagePair messagePair = readFromCircularBuffer(&publishedMessagesBuffer);

            ReleaseMutex(publishedMessagesMutex);

            // Notify subscribers and send the message to the storage service
            notifySubscribers(messagePair.topic, messagePair.message);
            char* storageMessage = format_struct_to_string(&messagePair);
            if (storageMessage) {
                printf("Formatted string: %s\n", storageMessage);
            }
            SendToStorage(storageMessage);

            // Free the memory for the topic and message
            free(messagePair.topic);
            free(messagePair.message);
            free(storageMessage);
        }
        else {
            ReleaseMutex(publishedMessagesMutex);
            Sleep(10); // Prevent busy-waiting
        }
    }
    return 0;
}




// Initialize the thread pool
void InitializeThreadPool()
{
    clientQueueMutex = CreateMutex(NULL, FALSE, NULL);
    for (int i = 0; i < MAX_THREADS; i++)
    {
        threadPool[i] = CreateThread(NULL, 0, WorkerFunction, NULL, 0, NULL);
    }
}

// Cleanup the thread pool
void CleanupThreadPool()
{
    for (int i = 0; i < MAX_THREADS; i++)
    {
        TerminateThread(threadPool[i], 0);
        CloseHandle(threadPool[i]);
    }
    CloseHandle(clientQueueMutex);
}

// Initialize the thread pool for processing messages
void InitializeMessageThreadPool() {
    for (int i = 0; i < MAX_MESSAGE_THREADS; i++) {
        messageThreadPool[i] = CreateThread(NULL, 0, MessageWorkerFunction, NULL, 0, NULL);
    }
}

// Cleanup the message processing thread pool
void CleanupMessageThreadPool() {
    for (int i = 0; i < MAX_MESSAGE_THREADS; i++) {
        TerminateThread(messageThreadPool[i], 0);
        CloseHandle(messageThreadPool[i]);
    }
}

char* format_struct_to_string(const TopicMessagePair* my_struct) {
    // Calculate the required length for the resulting string.
    // Include 1 for the colon and 1 for the null-terminator.
    size_t length = strlen(my_struct->topic) + strlen(my_struct->message) + 2;

    // Allocate memory for the formatted string.
    char* formatted_string = (char*)malloc(length);
    if (!formatted_string) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Format the string.
    sprintf(formatted_string, "%s:%s", my_struct->topic, my_struct->message);

    return formatted_string; // Caller is responsible for freeing this memory.
}

char* format_for_client(const char* topic, const char* message) {
    // Calculate the required length for the resulting string.
    // Include 1 for the colon and 1 for the null-terminator.
    size_t length = strlen(topic) + strlen(message) + 3;

    // Allocate memory for the formatted string.
    char* formatted_string = (char*)malloc(length);
    if (!formatted_string) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Format the string.
    sprintf(formatted_string, "%s: %s", topic, message);

    return formatted_string; // Caller is responsible for freeing this memory.
}
