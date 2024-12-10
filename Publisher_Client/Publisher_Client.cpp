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
void connect();
void Publish(SOCKET connectSocket, const char* topic, const char* message);
void flushInputBuffer();

int main(void)
{
    printf("-----Publisher Client-----\n");
    printf("Press any key to connect to the service (0 for exit) -> ");
    char c;
    scanf_s("%c", &c, 1);
    flushInputBuffer();
    if (c != '0') {
        connect();
    }

    printf("\nClosing the client...\n");
    Sleep(2000);
    return 0;
}

void connect() {
    SOCKET connectSocket = INVALID_SOCKET;
    char topic[BUFFER_SIZE];
    char messageContent[BUFFER_SIZE];

    int iResult;

    if (InitializeWindowsSockets() == false)
    {
        return;
    }

    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return;
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
        return;
    }

    while (true) {
        printf("Enter your topic (or type 'exit' to quit): ");
        if (fgets(topic, BUFFER_SIZE, stdin) == NULL) {
            printf("Error reading topic.\n");
            continue;
        }

        // Remove newline character from topic if present
        size_t topicLength = strlen(topic);
        if (topic[topicLength - 1] == '\n') {
            topic[topicLength - 1] = '\0';
        }

        // Exit condition
        if (strcmp(topic, "exit") == 0) {
            break;
        }

        // Input message content
        printf("Enter your message: ");
        if (fgets(messageContent, BUFFER_SIZE, stdin) == NULL) {
            printf("Error reading message.\n");
            continue;
        }

        // Remove newline character from messageContent if present
        size_t messageLength = strlen(messageContent);
        if (messageContent[messageLength - 1] == '\n') {
            messageContent[messageLength - 1] = '\0';
        }

        // Call Publish function to send the message
        Publish(connectSocket, topic, messageContent);
    }


    iResult = shutdown(connectSocket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return;
    }

    // Close connected socket
    closesocket(connectSocket);

    // Deinitialize WSA library
    WSACleanup();
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

void flushInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Discard characters until a newline or EOF
    }
}

void Publish(SOCKET connectSocket, const char* topic, const char* message) {
    char finalMessage[BUFFER_SIZE];
    int iResult;

    // Format the final message as PUBLISHER:topic:message
    snprintf(finalMessage, BUFFER_SIZE, "PUBLISHER:%s:%s", topic, message);
    // Send the formatted message to the server
    iResult = send(connectSocket, finalMessage, (int)strlen(finalMessage), 0);

    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        exit(1); // Exit the application if sending fails
    }

    printf("Message successfully sent to topic '%s'. Total bytes sent: %d\n", topic, iResult);
}


