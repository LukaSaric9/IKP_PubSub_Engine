#define _CRT_SECURE_NO_WARNINGS
#include "MessageProcessing.h"

MessageRecord* messageListHead = NULL;

void SaveMessage(const char* topic, const char* message, int priority) {
    // Allocate memory for the new message record
    MessageRecord* newMessage = (MessageRecord*)malloc(sizeof(MessageRecord));

    // Allocate and copy topic and message
    newMessage->topic = _strdup(topic);
    newMessage->message = _strdup(message);
    newMessage->priority = priority;
    newMessage->timestamp = time(NULL);  // Get the current time
    newMessage->next = NULL;

    // Add the new message to the linked list
    if (messageListHead == NULL) {
        messageListHead = newMessage;
    }
    else {
        MessageRecord* temp = messageListHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newMessage;
    }

    // Enhanced save message output with ASCII-safe priority display
    printf("\n*** MESSAGE RECEIVED AND SAVED! ***\n");
    printf("+------------------------------------------------+\n");
    printf("| Topic:    %s\n", topic);
    printf("| Priority: %s %s (%d)\n", getPriorityIcon(priority), getPriorityName(priority), priority);
    printf("| Message:  %s\n", message);
    printf("| Time:     %s", ctime(&newMessage->timestamp));
    printf("+------------------------------------------------+\n");
}

DWORD WINAPI ReceiveMessages(LPVOID lpParam) {
    SOCKET connectSocket = (SOCKET)lpParam;
    char recvBuffer[BUFFER_SIZE];
    int bytesReceived;

    while (true) {
        // Receive data from the server
        bytesReceived = recv(connectSocket, recvBuffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            recvBuffer[bytesReceived] = '\0'; // Null-terminate the message
            printf("\n>> Raw message from server: %s\n", recvBuffer);

            // Parse message format: Check if it includes priority
            // New format: PRIORITY:topic:message
            // Old format: topic:message
            
            const char* firstDelimiter = strchr(recvBuffer, ':');
            if (firstDelimiter != NULL) {
                // Check if first part is a priority number
                char priorityStr[10];
                size_t priorityLength = firstDelimiter - recvBuffer;
                
                if (priorityLength < sizeof(priorityStr)) {
                    strncpy_s(priorityStr, sizeof(priorityStr), recvBuffer, priorityLength);
                    priorityStr[priorityLength] = '\0';
                    
                    // Try to parse as priority
                    char* endPtr;
                    long priority = strtol(priorityStr, &endPtr, 10);
                    
                    if (*endPtr == '\0' && priority >= PRIORITY_HIGH && priority <= PRIORITY_LOW) {
                        // New format with priority: PRIORITY:topic:message
                        const char* secondDelimiter = strchr(firstDelimiter + 1, ':');
                        if (secondDelimiter != NULL) {
                            // Extract topic
                            size_t topicLength = secondDelimiter - (firstDelimiter + 1);
                            char topic[BUFFER_SIZE];
                            strncpy_s(topic, sizeof(topic), firstDelimiter + 1, topicLength);
                            topic[topicLength] = '\0';
                            
                            // Extract message
                            const char* messageContent = secondDelimiter + 1;
                            
                            // Save with actual priority from server
                            SaveMessage(topic, messageContent, (int)priority);
                            continue;
                        }
                    }
                }
                
                // Fall back to old format: topic:message
                size_t topicLength = firstDelimiter - recvBuffer;
                char topic[BUFFER_SIZE];
                strncpy_s(topic, sizeof(topic), recvBuffer, topicLength);
                topic[topicLength] = '\0';

                // Default to MEDIUM priority for old format
                SaveMessage(topic, firstDelimiter + 1, PRIORITY_MEDIUM);
            }
            else {
                printf("!! Invalid message format received.\n");
            }
        }
        else if (bytesReceived == 0) {
            printf(">> Server disconnected.\n");
            break;
        }
        else {
            printf("!! recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}

void SearchMessages(const char* searchTopic) {
    printf("\n*** SEARCHING MESSAGES FOR TOPIC: %s ***\n", searchTopic);
    printf("==================================================\n");
    
    MessageRecord* temp = messageListHead;
    int count = 0;
    
    while (temp != NULL) {
        if (strcmp(temp->topic, searchTopic) == 0) {
            count++;
            printf(">> Message #%d:\n", count);
            printf("   Priority: %s %s (%d)\n", getPriorityIcon(temp->priority), getPriorityName(temp->priority), temp->priority);
            printf("   Content:  %s\n", temp->message);
            printf("   Time:     %s\n", ctime(&temp->timestamp));
        }
        temp = temp->next;
    }
    
    if (count == 0) {
        printf("!! No messages found for topic: %s\n", searchTopic);
    } else {
        printf("*** Found %d message(s) for topic: %s\n", count, searchTopic);
    }
    printf("==================================================\n\n");
}

void ReadAllMessages() {
    printf("\n*** ALL STORED MESSAGES ***\n");
    printf("==================================================\n");
    
    MessageRecord* temp = messageListHead;
    int count = 0;
    
    while (temp != NULL) {
        count++;
        printf(">> Message #%d:\n", count);
        printf("   Topic:    %s\n", temp->topic);
        printf("   Priority: %s %s (%d)\n", getPriorityIcon(temp->priority), getPriorityName(temp->priority), temp->priority);
        printf("   Content:  %s\n", temp->message);
        printf("   Time:     %s\n", ctime(&temp->timestamp));
        temp = temp->next;
    }
    
    if (count == 0) {
        printf("!! No messages stored yet.\n");
    } else {
        printf("*** Total messages stored: %d\n", count);
    }
    printf("==================================================\n\n");
}

void CleanupMessages() {
    printf(">> Cleaning up stored messages...\n");
    MessageRecord* temp = messageListHead;
    int count = 0;
    
    while (temp != NULL) {
        MessageRecord* toDelete = temp;
        temp = temp->next;
        free(toDelete->topic);
        free(toDelete->message);
        free(toDelete);
        count++;
    }
    messageListHead = NULL;
    printf("*** Cleaned up %d messages.\n", count);
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


