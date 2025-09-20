#include "thread_pool.h"
#include "pubsub.h"

// Thread pool and client queue definitions - only threading concerns
HANDLE threadPool[MAX_THREADS];
HANDLE clientQueueMutex;
SOCKET clientQueue[MAX_THREADS];
int queueCount = 0;
HANDLE messageThreadPool[MAX_MESSAGE_THREADS];

// Worker function for handling client connections
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
            ProcessClientMessage(clientSocket); // Now calls PubSub function
        }
        else {
            Sleep(10); // Prevent busy-waiting
        }
    }
    return 0;
}

// Enhanced worker function for processing messages with priority support 
DWORD WINAPI MessageWorkerFunction(LPVOID lpParam) {
    while (true) {
        WaitForSingleObject(publishedMessagesMutex, INFINITE);


        // Check if the buffer contains any messages
        if (publishedMessagesBuffer.size > 0) {
            // Use priority-aware reading - highest priority messages first!
            PriorityTopicMessagePair messagePair = readHighestPriorityFromCircularBuffer(&publishedMessagesBuffer);
            ReleaseMutex(publishedMessagesMutex);

            // Use PubSub functions for processing
            printf("Processing %s priority message: %s:%s\n", 
                   getPriorityName(messagePair.priority), messagePair.topic, messagePair.message);
            
            // Send priority-aware message to subscribers
            notifySubscribersWithPriority(messagePair.topic, messagePair.message, messagePair.priority);
            
            char* storageMessage = format_priority_struct_to_string(&messagePair);
            if (storageMessage) {
                SendToStorage(storageMessage);
                free(storageMessage);
            }

            // Free the memory for the topic and message
            free(messagePair.topic);
            free(messagePair.message);
        }
        else {
            ReleaseMutex(publishedMessagesMutex);
            Sleep(10); // Prevent busy-waiting
        }
    }
    return 0;
}

// Initialize the thread pool
void InitializeThreadPool() {
    clientQueueMutex = CreateMutex(NULL, FALSE, NULL);
    for (int i = 0; i < MAX_THREADS; i++) {
        threadPool[i] = CreateThread(NULL, 0, WorkerFunction, NULL, 0, NULL);
    }
}

// Cleanup the thread pool
void CleanupThreadPool() {
    for (int i = 0; i < MAX_THREADS; i++) {
        TerminateThread(threadPool[i], 0);
        CloseHandle(threadPool[i]);
    }
    CloseHandle(clientQueueMutex);
}

// Initialize the message processing thread pool
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

// Add client socket to the processing queue
void AddClientToQueue(SOCKET clientSocket) {
    WaitForSingleObject(clientQueueMutex, INFINITE);
    if (queueCount < MAX_THREADS) {
        clientQueue[queueCount++] = clientSocket;
    }
    ReleaseMutex(clientQueueMutex);
}