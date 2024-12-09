#define _CRT_SECURE_NO_WARNINGS
#include "MessageProcessing.h"

MessageRecord* messageListHead = NULL;

void SaveMessage(const char* topic, const char* message) {
    // Allocate memory for the new message record
    MessageRecord* newMessage = (MessageRecord*)malloc(sizeof(MessageRecord));

    // Allocate and copy topic and message
    newMessage->topic = _strdup(topic);
    newMessage->message = _strdup(message);
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

    printf("Saved message: %s: %s at %s", topic, message, ctime(&newMessage->timestamp));
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
            printf("\nMessage received from server: %s\n", recvBuffer);

            // Assume the message is in the format topic:message
            const char* delimiter = strchr(recvBuffer, ':');
            if (delimiter != NULL) {
                size_t topicLength = delimiter - recvBuffer;
                char topic[BUFFER_SIZE];
                strncpy_s(topic, topicLength + 1, recvBuffer, topicLength);
                topic[topicLength] = '\0';

                // Save the message
                SaveMessage(topic, delimiter + 1);
            }
            else {
                printf("Invalid message format received.\n");
            }
        }
        else if (bytesReceived == 0) {
            printf("Server disconnected.\n");
            break;
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}

void SearchMessages(const char* searchTopic) {
    printf("Messages for topic: %s\n", searchTopic);
    MessageRecord* temp = messageListHead;
    while (temp != NULL) {
        if (strcmp(temp->topic, searchTopic) == 0) {
            printf("Message: %s\t at %s\n", temp->message, ctime(&temp->timestamp));
        }
        temp = temp->next;
    }
}

void ReadAllMessages() {
    MessageRecord* temp = messageListHead;
    while (temp != NULL) {
        printf("Topic: %s\tMessage: %s\t at %s\n", temp->topic, temp->message, ctime(&temp->timestamp));
        temp = temp->next;
    }
    printf("\n");
}

void CleanupMessages() {
    MessageRecord* temp = messageListHead;
    while (temp != NULL) {
        MessageRecord* toDelete = temp;
        temp = temp->next;
        free(toDelete->topic);
        free(toDelete->message);
        free(toDelete);
    }
    messageListHead = NULL;
}


