#include "thread_pool.h"  


// Thread pool and client queue definitions
HANDLE threadPool[MAX_THREADS];
HANDLE clientQueueMutex;
SOCKET clientQueue[MAX_THREADS];
int queueCount = 0;

std::vector<std::string> topics;
std::mutex topicsMutex;

void ProcessClientMessage(SOCKET clientSocket) {
    char recvBuffer[BUFFER_LENGTH];
    int bytesReceived;

    while (true) {
        bytesReceived = recv(clientSocket, recvBuffer, BUFFER_LENGTH - 1, 0);
        if (bytesReceived > 0) {
            recvBuffer[bytesReceived] = '\0'; // Null-terminate the message
            printf("Received message: %s\n", recvBuffer);

            if (strncmp(recvBuffer, "SUBSCRIBER:", 11) == 0) {
                const char* topic = recvBuffer + 11; // Extract the topic
                std::lock_guard<std::mutex> lock(topicsMutex);
                topics.push_back(topic);
                printf("Subscrbier just added a topic: %s\n", topic);
            }
            else if (strncmp(recvBuffer, "PUBLISHER:", 10) == 0) {
                const char* message = recvBuffer + 10; // Extract the message
                printf("Publisher sent message: %s\n", message);
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