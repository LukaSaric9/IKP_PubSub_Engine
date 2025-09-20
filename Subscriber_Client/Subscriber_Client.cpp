#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <ws2tcpip.h>
#include <time.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 7000
#define BUFFER_SIZE 1024

// Priority definitions (to detect from message content)
#define PRIORITY_HIGH 0
#define PRIORITY_MEDIUM 1
#define PRIORITY_LOW 2

bool InitializeWindowsSockets();
void connect();
void flushInputBuffer();
DWORD WINAPI ReceiveMessages(LPVOID lpParam);
void Subscribe(SOCKET connectSocket, const char* topic);
const char* getPriorityName(int priority);
const char* getPriorityIcon(int priority);
void displayMessage(const char* topic, const char* message, int estimatedPriority);

void setupConsole();
void displayMessageWithActualPriority(const char* topic, const char* message, int actualPriority);

int main()
{
    setupConsole();
    
    printf("+============================================+\n");
    printf("|       Subscriber Client with Priority     |\n");
    printf("|         Real-time Message Monitoring      |\n");
    printf("+============================================+\n");
    printf("Press any key to connect to the service (0 for exit) -> ");
    char c;
    scanf_s("%c", &c, 1);
    flushInputBuffer();
    if (c != '0') {
        connect();
    }

    printf("\n>> Closing the client...\n");
    Sleep(2000);
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

    int iResult;

    char dataBuffer[BUFFER_SIZE];

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

    // Create a thread to handle receiving messages from the server
    HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessages, (LPVOID)connectSocket, 0, NULL);
    if (receiveThread == NULL)
    {
        printf("Failed to create receive thread. Error: %ld\n", GetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return;
    }

    printf("\n+============================================+\n");
    printf("|              SUBSCRIPTION PANEL           |\n");
    printf("|                                            |\n");
    printf("| * Enter topics to monitor messages        |\n");
    printf("| [H] HIGH priority messages shown first    |\n");
    printf("| [M] MEDIUM priority messages shown second |\n");
    printf("| [L] LOW priority messages shown last      |\n");
    printf("|                                            |\n");
    printf("| Type 'exit' to quit the subscriber        |\n");
    printf("+============================================+\n\n");

    while (true) {
        Sleep(100);
        printf(">> Enter topic to subscribe (or type 'exit' to quit): ");
        gets_s(dataBuffer, BUFFER_SIZE);

        // Exit condition
        if (strcmp(dataBuffer, "exit") == 0) {
            printf("\n>> Disconnecting from server...\n");
            shutdown(connectSocket, SD_BOTH);
            closesocket(connectSocket);
            WSACleanup();
            return;
        }

        // Call Subscribe function to subscribe to the entered topic
        Subscribe(connectSocket, dataBuffer);
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

// Enhanced thread function for receiving and displaying messages with actual priority
DWORD WINAPI ReceiveMessages(LPVOID lpParam)
{
    SOCKET connectSocket = (SOCKET)lpParam;
    char recvBuffer[BUFFER_SIZE];
    int bytesReceived;

    printf("\n*** LIVE MESSAGE MONITOR - Waiting for messages...\n");
    printf("=======================================================\n\n");

    while (true)
    {
        // Receive data from the server
        bytesReceived = recv(connectSocket, recvBuffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0)
        {
            recvBuffer[bytesReceived] = '\0'; // Null-terminate the message
            
            // Parse the message - New format: [PRIORITY]topic: message
            // Example: [0]nba: jordan means HIGH priority message
            
            int actualPriority = PRIORITY_MEDIUM; // Default
            char* messageStart = recvBuffer;
            
            // Check if message starts with priority indicator [N]
            if (recvBuffer[0] == '[') {
                char* closeBracket = strchr(recvBuffer, ']');
                if (closeBracket != NULL) {
                    // Extract priority number
                    char priorityStr[10];
                    size_t priorityLength = closeBracket - recvBuffer - 1;
                    if (priorityLength < sizeof(priorityStr)) {
                        strncpy_s(priorityStr, sizeof(priorityStr), recvBuffer + 1, priorityLength);
                        priorityStr[priorityLength] = '\0';
                        
                        // Parse priority
                        char* endPtr;
                        long priority = strtol(priorityStr, &endPtr, 10);
                        if (*endPtr == '\0' && priority >= PRIORITY_HIGH && priority <= PRIORITY_LOW) {
                            actualPriority = (int)priority;
                            messageStart = closeBracket + 1; // Skip past [N]
                        }
                    }
                }
            }
            
            // Parse the remaining message: topic: message
            const char* delimiter = strstr(messageStart, ": ");
            if (delimiter != NULL) {
                // Extract topic
                size_t topicLength = delimiter - messageStart;
                char topic[BUFFER_SIZE];
                strncpy_s(topic, sizeof(topic), messageStart, topicLength);
                topic[topicLength] = '\0';
                
                // Extract message content (skip ": ")
                const char* messageContent = delimiter + 2;
                
                // Display the message with ACTUAL priority (not estimated)
                displayMessageWithActualPriority(topic, messageContent, actualPriority);
            } else {
                // Fallback: display raw message if parsing fails
                printf(">> Raw message: %s\n", recvBuffer);
            }
        }
        else if (bytesReceived == 0)
        {
            printf("\n>> Server disconnected.\n");
            break;
        }
        else
        {
            printf("!! recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}

void flushInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Discard characters until a newline or EOF
    }
}

void Subscribe(SOCKET connectSocket, const char* topic) {
    char message[BUFFER_SIZE];
    int iResult;

    // Prepend SUBSCRIBER identifier to the topic
    snprintf(message, BUFFER_SIZE, "SUBSCRIBER:%s", topic);

    // Send the subscription message to the server
    iResult = send(connectSocket, message, (int)strlen(message), 0);

    if (iResult == SOCKET_ERROR) {
        printf("!! send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        exit(1); // Exit the application if sending fails
    }

    // Confirmation message with visual enhancement
    printf("\n*** SUBSCRIPTION SUCCESSFUL! ***\n");
    printf("+-------------------------------------------+\n");
    printf("| >> Now monitoring topic: '%s'\n", topic);
    printf("| >> You'll receive all messages for this topic\n");
    printf("+-------------------------------------------+\n\n");
}

void displayMessage(const char* topic, const char* message, int estimatedPriority) {
    // Get current time
    time_t now = time(NULL);
    char timeStr[64];
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    
    // Display message with priority indicators and beautiful formatting
    printf("+==========================================================+\n");
    printf("| *** NEW MESSAGE RECEIVED ***                            |\n");
    printf("+==========================================================+\n");
    printf("| Topic:    %s\n", topic);
    printf("| Priority: %s %s (estimated)\n", 
           getPriorityIcon(estimatedPriority), 
           getPriorityName(estimatedPriority));
    printf("| Content:  %s\n", message);
    printf("| Time:     %s\n", timeStr);
    printf("+==========================================================+\n\n");
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

// New function to display message with actual priority (not estimated)
void displayMessageWithActualPriority(const char* topic, const char* message, int actualPriority) {
    // Get current time
    time_t now = time(NULL);
    char timeStr[64];
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    
    // Display message with ACTUAL priority from server
    printf("+==========================================================+\n");
    printf("| *** NEW MESSAGE RECEIVED ***                            |\n");
    printf("+==========================================================+\n");
    printf("| Topic:    %s\n", topic);
    printf("| Priority: %s %s \n", 
           getPriorityIcon(actualPriority), 
           getPriorityName(actualPriority));
    printf("| Content:  %s\n", message);
    printf("| Time:     %s\n", timeStr);
    printf("+==========================================================+\n\n");
}


