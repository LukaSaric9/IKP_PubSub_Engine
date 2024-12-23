#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <ws2tcpip.h>
#include "MessageProcessing.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 7000
#define BUFFER_SIZE 1024
#define STORAGE_IDENTIFIER "STORAGE"

bool InitializeWindowsSockets();
DWORD WINAPI ReceiveMessages(LPVOID lpParam);

int main(void)
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
        return 1;
    }

    // Send the STORAGE identifier to recognize this client as a storage service
    char message[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "%s", STORAGE_IDENTIFIER);
    iResult = send(connectSocket, message, (int)strlen(message), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessages, (LPVOID)connectSocket, 0, NULL);
    if (receiveThread == NULL)
    {
        printf("Failed to create receive thread. Error: %ld\n", GetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("-----Storage Service-----\n");
    while (true) {
        printf("\nChoose an option:\n1. Display all messages\n2. Search by topic\n3. Close The Storage Service\n");
        char number = 0;
        number = getchar();
        fflush(stdin);
        switch (number) {
            case '1':
                ReadAllMessages();
                break;
            case '2':
                printf("Enter your topic -> ");
                char topic[30];
                if (scanf_s("%29s", topic, (unsigned)_countof(topic)) != 1) {
                    printf("Failed to read topic.\n");
                    break; // Exit if input fails
                }
                fflush(stdin);
                SearchMessages(topic);
                break;
            case '3':
                CleanupMessages();
                iResult = shutdown(connectSocket, SD_BOTH);
                closesocket(connectSocket);
                WSACleanup();
                return 0;
            default:
                printf("You must choose a valid option!\n");
                break;               
        }

    }

    // Wait for the receive thread to finish
    WaitForSingleObject(receiveThread, INFINITE);

    // Cleanup all messages when done
    CleanupMessages();

    iResult = shutdown(connectSocket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

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



