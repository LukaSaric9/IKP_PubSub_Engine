#include "thread_pool.h"  
#include "hashmap.h"
#include "buffer.h"


// Thread pool and client queue definitions
HANDLE threadPool[MAX_THREADS];
HANDLE clientQueueMutex;
SOCKET clientQueue[MAX_THREADS];
int queueCount = 0;


// Define the global data structures
HashMap topicSubscribers; // Use the custom HashMap for topic-subscriber mapping
DynamicBuffer publishedMessagesBuffer;
HANDLE publishedMessagesMutex; // Mutex for publishedMessages

void notifySubscribers(const char* topic, const char* message);

// Initialize global structures and mutex
void InitializeGlobalData() {
    initializeHashMapWithMutex(&topicSubscribers);
    publishedMessagesMutex = CreateMutex(NULL, FALSE, NULL); // Mutex for publishedMessages
}

// Clean up global structures
void CleanupGlobalData() {
    freeHashMapWithMutex(&topicSubscribers);
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

        strncpy_s(content, delimeter + 1, sizeof(content - 1));
        content[sizeof(content) - 1] = '\0';

        WaitForSingleObject(publishedMessagesMutex, INFINITE);
        // ovde treba upis u buffer koji jos nemamo
        storeTopicMessage(&publishedMessagesBuffer, topic, content); // Store the topic-message pair
        printBufferContents(&publishedMessagesBuffer);              // Optional: Debug output
        ReleaseMutex(publishedMessagesMutex);

        //zatim obavestavanje svih klijenata koji su subscribovani
        notifySubscribers(topic, content);
    } 
    else {
        printf("Invalid publisher message format.\n");
    }

}

void notifySubscribers(const char* topic, const char* message) {
    printf("Notifying subscribers for topic: '%s'\n", topic);

    // Lock the hash map to safely access subscribers
    SubscriberNode* subscribers = getSubscribersWithLock(&topicSubscribers, topic);

    while (subscribers) {
        SOCKET subscriberSocket = subscribers->socket;

        // Send the message to the subscriber
        int sendResult = send(subscriberSocket, message, strlen(message), 0);
        if (sendResult == SOCKET_ERROR) {
            printf("Failed to send message to subscriber. Error: %d\n", WSAGetLastError());
        }
        else {
            printf("Message sent to subscriber (socket: %lld): '%s'\n", (long long)subscriberSocket, message);
        }

        subscribers = subscribers->next;
    }

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
            printf("Received message: %s\n", recvBuffer);

            if (strncmp(recvBuffer, "PUBLISHER:", 10) == 0) {
                ProcessPublisherMessage(clientSocket, recvBuffer + 10);
            }
            else if (strncmp(recvBuffer, "SUBSCRIBER:", 11) == 0) {
                ProcessSubscriberMessage(clientSocket, recvBuffer + 11);
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