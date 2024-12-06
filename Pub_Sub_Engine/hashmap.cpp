#define _CRT_SECURE_NO_WARNINGS
#include "hashmap.h"

HANDLE hashMapMutex;
const char* concatenate(const char* str1, const char* str2);

unsigned int hashFunction(const char* topic) {
    unsigned int hash = 0;
    while (*topic) {
        hash = (hash * 31) + *topic++;
    }
    return hash % HASH_MAP_SIZE;
}

void initializeHashMapWithMutex(HashMap* map) {
    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        map->buckets[i] = NULL;
    }
    hashMapMutex = CreateMutex(NULL, FALSE, NULL); // Initialize the mutex
}

void insertIntoHashMapWithLock(HashMap* map, const char* topic, SOCKET socket) {
    WaitForSingleObject(hashMapMutex, INFINITE); // Lock mutex

    unsigned int index = hashFunction(topic);
    HashMapNode* current = map->buckets[index];

    // Traverse the hash map bucket to find the topic
    while (current) {
        if (strcmp(current->topic, topic) == 0) {
            // Check if the subscriber is already subscribed
            SubscriberNode* subscriber = current->subscribers;
            while (subscriber) {
                if (subscriber->socket == socket) {
                    printf("Socket %d is already subscribed to topic '%s'.\n", socket, topic);
                    const char* message = concatenate("You are already subscribed to ",topic);
                    int sendResult = send(socket, message, strlen(message), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Failed to send message to subscriber. Error: %d\n", WSAGetLastError());
                    }
                    free((void*)message);

                    ReleaseMutex(hashMapMutex); // Release mutex
                    return; // Exit function without adding the socket
                }
                subscriber = subscriber->next;
            }

            // Add the subscriber to the topic
            SubscriberNode* newSubscriber = (SubscriberNode*)malloc(sizeof(SubscriberNode));
            newSubscriber->socket = socket;
            newSubscriber->next = current->subscribers;
            current->subscribers = newSubscriber;
            printf("Subscriber added to topic: %s\n", topic);
            const char* message = concatenate("You succesfully subscribed the topic: ",topic);
            int sendResult = send(socket, message, strlen(message), 0);
            if (sendResult == SOCKET_ERROR) {
                printf("Failed to send message to subscriber. Error: %d\n", WSAGetLastError());
            }
            free((void*)message);

            ReleaseMutex(hashMapMutex); // Release mutex
            return;
        }
        current = current->next;
    }

    // If topic doesn't exist, create a new node
    HashMapNode* newNode = (HashMapNode*)malloc(sizeof(HashMapNode));
    strcpy_s(newNode->topic, topic);
    newNode->subscribers = (SubscriberNode*)malloc(sizeof(SubscriberNode));
    newNode->subscribers->socket = socket;
    newNode->subscribers->next = NULL;
    newNode->next = map->buckets[index];
    map->buckets[index] = newNode;
    printf("Subscriber added to topic: %s\n", topic);
    const char* message = concatenate("You succesfully subscribed the topic: ", topic);
    int sendResult = send(socket, message, strlen(message), 0);
    if (sendResult == SOCKET_ERROR) {
        printf("Failed to send message to subscriber. Error: %d\n", WSAGetLastError());
    }
    free((void*)message);


    ReleaseMutex(hashMapMutex); // Release mutex
}


SubscriberNode* getSubscribersWithLock(HashMap* map, const char* topic) {
    WaitForSingleObject(hashMapMutex, INFINITE); // Lock mutex

    unsigned int index = hashFunction(topic);
    HashMapNode* current = map->buckets[index];
    while (current) {
        if (strcmp(current->topic, topic) == 0) {
            SubscriberNode* subscribers = current->subscribers;

            ReleaseMutex(hashMapMutex); // Release mutex
            return subscribers;
        }
        current = current->next;
    }

    ReleaseMutex(hashMapMutex); // Release mutex
    return NULL; // Topic not found
}

void freeHashMapWithMutex(HashMap* map) {
    WaitForSingleObject(hashMapMutex, INFINITE); // Lock mutex

    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        HashMapNode* current = map->buckets[i];
        while (current) {
            HashMapNode* temp = current;
            current = current->next;

            SubscriberNode* subscriber = temp->subscribers;
            while (subscriber) {
                SubscriberNode* subTemp = subscriber;
                subscriber = subscriber->next;
                free(subTemp);
            }
            free(temp);
        }
        map->buckets[i] = NULL;
    }

    ReleaseMutex(hashMapMutex); // Release mutex
    CloseHandle(hashMapMutex); // Destroy mutex
}

const char* concatenate(const char* str1, const char* str2) {
    // Calculate the length of the two strings
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    // Allocate memory for the concatenated string (+1 for the null terminator)
    char* result = (char*)malloc(len1 + len2 + 1);
    if (result == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    // Copy the first string into the result
    strcpy(result, str1);

    // Append the second string
    strcat(result, str2);

    // Return the result (caller is responsible for freeing the memory)
    return result;
}
