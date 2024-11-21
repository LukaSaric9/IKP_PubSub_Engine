#include "thread_pool.h"
/*
void InitializeThreadPool()
{
    clientQueueMutex = CreateMutex(NULL, FALSE, NULL);
    for (int i = 0; i < MAX_THREADS; i++)
    {
        threadPool[i] = CreateThread(NULL, 0, WorkerFunction, NULL, 0, NULL);
    }
}

void CleanupThreadPool()
{
    for (int i = 0; i < MAX_THREADS; i++)
    {
        TerminateThread(threadPool[i], 0);
        CloseHandle(threadPool[i]);
    }
    CloseHandle(clientQueueMutex);
}

DWORD WINAPI WorkerFunction(LPVOID lpParam)
{
    while (true)
    {
        SOCKET clientSocket = INVALID_SOCKET;

        // Wait for a client in the queue
        WaitForSingleObject(clientQueueMutex, INFINITE);
        if (queueCount > 0)
        {
            clientSocket = clientQueue[--queueCount];
        }
        ReleaseMutex(clientQueueMutex);

        if (clientSocket != INVALID_SOCKET)
        {
            char recvbuf[BUFFER_LENGTH];
            int iResult;

            do
            {
                iResult = recv(clientSocket, recvbuf, BUFFER_LENGTH, 0);
                if (iResult > 0)
                {
                    recvbuf[iResult] = '\0';
                    printf("Client sent: %s\n", recvbuf);
                }
                else if (iResult == 0)
                {
                    printf("Connection with client closed.\n");
                }
                else
                {
                    printf("recv failed with error: %d\n", WSAGetLastError());
                }
            } while (iResult > 0);

            closesocket(clientSocket); // Close the client socket after processing
        }
        else
        {
            Sleep(10); // Prevent busy-waiting
        }
    }
    return 0;
}
*/