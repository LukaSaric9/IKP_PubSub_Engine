#include "hashmap.h"

HANDLE hashMapMutex;

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
