#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include "common.h"
#include "thread_pool.h"
#include "hashmap.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


bool InitializeWindowsSockets();

int main()
{
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET acceptedSocket = INVALID_SOCKET;
    int iResult;

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(DEFAULT_PORT);

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Server socket is set to listening mode. Waiting for new connection requests.\n");
    
    InitializeGlobalData();
    // Initialize thread pool
    InitializeThreadPool();
    InitializeMessageThreadPool();

    do
    {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        acceptedSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            continue;
        }

        printf("\nNew client connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        WaitForSingleObject(clientQueueMutex, INFINITE);
        if (queueCount < MAX_THREADS)
        {
            clientQueue[queueCount++] = acceptedSocket;
        }
        else
        {
            printf("Server is busy. Closing connection.\n");
            closesocket(acceptedSocket);
        }
        ReleaseMutex(clientQueueMutex);

    } while (true);

    CleanupThreadPool();
    CleanupGlobalData();
    CleanupMessageThreadPool();

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}
