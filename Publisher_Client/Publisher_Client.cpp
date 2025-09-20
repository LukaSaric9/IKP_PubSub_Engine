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

// Priority definitions
#define PRIORITY_HIGH 0
#define PRIORITY_MEDIUM 1
#define PRIORITY_LOW 2

bool InitializeWindowsSockets();
void connect();
void Publish(SOCKET connectSocket, const char* topic, const char* message, int priority);
void flushInputBuffer();
const char* getPriorityName(int priority);
const char* getPriorityIcon(int priority);
void setupConsole();

int main(void)
{
    setupConsole();
    
    printf("+============================================+\n");
    printf("|       Publisher Client with Priority      |\n");
    printf("+============================================+\n");
    printf("Press any key to connect to the service (0 for exit) -> ");
    char c;
    scanf_s("%c", &c, 1);
    flushInputBuffer();
    if (c != '0') {
        connect();
    }

    printf("\n>> Closing the client...\n");
    Sleep(500);
    return 0;
}

void setupConsole() {
    // Set console output to UTF-8 to handle special characters better
    SetConsoleOutputCP(CP_UTF8);
    
    // Try to enable virtual terminal processing for better character support
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void connect() {
    SOCKET connectSocket = INVALID_SOCKET;
    char topic[BUFFER_SIZE];
    char messageContent[BUFFER_SIZE];
    int priority;

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

    printf("\n+==============================================+\n");
    printf("|            Priority Message Publisher       |\n");
    printf("+==============================================+\n");
    printf("| Priority Levels:                            |\n");
    printf("|   0 = HIGH    [H] (Processed first)         |\n");
    printf("|   1 = MEDIUM  [M] (Processed second)        |\n");
    printf("|   2 = LOW     [L] (Processed last)          |\n");
    printf("+==============================================+\n\n");

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

        // Input message priority
        printf("Select message priority (0=HIGH, 1=MEDIUM, 2=LOW): ");
        if (scanf_s("%d", &priority) != 1) {
            printf("!! Invalid priority input. Using MEDIUM priority.\n");
            priority = PRIORITY_MEDIUM;
            flushInputBuffer();
        } else {
            flushInputBuffer();
        }

        // Validate priority range
        if (priority < PRIORITY_HIGH || priority > PRIORITY_LOW) {
            printf("!! Invalid priority range. Using MEDIUM priority.\n");
            priority = PRIORITY_MEDIUM;
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

        // Call Publish function to send the message with priority
        Publish(connectSocket, topic, messageContent, priority);
        
        // Add a delay to prevent timing conflicts with subscriber displays
        printf(">> Allowing time for message processing...\n");
        Sleep(1500); // 1.5 second delay to let subscribers process and display the message
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

void Publish(SOCKET connectSocket, const char* topic, const char* message, int priority) {
    char finalMessage[BUFFER_SIZE];
    int iResult;

    // Format the final message as PUBLISHER:PRIORITY:topic:message
    snprintf(finalMessage, BUFFER_SIZE, "PUBLISHER:%d:%s:%s", priority, topic, message);
    
    // Send the formatted message to the server
    iResult = send(connectSocket, finalMessage, (int)strlen(finalMessage), 0);

    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        exit(1); // Exit the application if sending fails
    }

    // Enhanced console output with priority information and ASCII-safe visual indicators
    printf("\n*** MESSAGE SENT SUCCESSFULLY! ***\n");
    printf("+------------------------------------------------+\n");
    printf("| Topic:    '%s'\n", topic);
    printf("| Priority: %s %s (%d)\n", getPriorityIcon(priority), getPriorityName(priority), priority);
    printf("| Message:  '%s'\n", message);
    printf("| Bytes:    %d bytes sent\n", iResult);
    printf("+------------------------------------------------+\n");
    
    // Short delay after display to ensure clean output before returning to main loop
    Sleep(200); // Half second delay for clean console output
}

const char* getPriorityName(int priority) {
    switch (priority) {
        case PRIORITY_HIGH: return "HIGH";
        case PRIORITY_MEDIUM: return "MEDIUM";
        case PRIORITY_LOW: return "LOW";
        default: return "UNKNOWN";
    }
}

const char* getPriorityIcon(int priority) {
    switch (priority) {
        case PRIORITY_HIGH: return "[H]";
        case PRIORITY_MEDIUM: return "[M]";
        case PRIORITY_LOW: return "[L]";
        default: return "[?]";
    }
}


