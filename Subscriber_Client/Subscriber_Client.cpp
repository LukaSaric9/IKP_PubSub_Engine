#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 7000
#define BUFFER_SIZE 1024

bool InitializeWindowsSockets();
DWORD WINAPI ReceiveMessages(LPVOID lpParam);

int main()
{
    SOCKET connectSocket = INVALID_SOCKET;

    int iResult;

    char dataBuffer[BUFFER_SIZE];

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    serverAddress.sin_port = htons(SERVER_PORT);


    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }

    // Create a thread to handle receiving messages from the server
    HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessages, (LPVOID)connectSocket, 0, NULL);
    if (receiveThread == NULL)
    {
        printf("Failed to create receive thread. Error: %ld\n", GetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        Sleep(100);
        printf("Enter topic to subscribe: ");
        gets_s(dataBuffer, BUFFER_SIZE);

        // Prepend SUBSCRIBER identifier
        char message[BUFFER_SIZE];
        snprintf(message, BUFFER_SIZE, "SUBSCRIBER:%s", dataBuffer);

        iResult = send(connectSocket, message, (int)strlen(message), 0);

        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }

        printf("Topic successfully sent. Total bytes: %ld\n", iResult);
    }

    // Wait for the receive thread to finish
    WaitForSingleObject(receiveThread, INFINITE);

    iResult = shutdown(connectSocket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // For demonstration purpose
    printf("\nPress any key to exit: ");
    _getch();


    // Close connected socket
    closesocket(connectSocket);

    // Deinitialize WSA library
    WSACleanup();
    return 0;
}


bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

// Thread function for receiving messages from the server
DWORD WINAPI ReceiveMessages(LPVOID lpParam)
{
    SOCKET connectSocket = (SOCKET)lpParam;
    char recvBuffer[BUFFER_SIZE];
    int bytesReceived;

    while (true)
    {
        // Receive data from the server
        bytesReceived = recv(connectSocket, recvBuffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0)
        {
            recvBuffer[bytesReceived] = '\0'; // Null-terminate the message
            printf("\nMessage received from server: %s\n", recvBuffer);
        }
        else if (bytesReceived == 0)
        {
            printf("Server disconnected.\n");
            break;
        }
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}


