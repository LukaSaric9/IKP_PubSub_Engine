#ifndef HASHMAP_H
#define HASHMAP_H

#include "common.h"

#define HASH_MAP_SIZE 256
#define TOPIC_SIZE 64

extern HANDLE hashMapMutex;

// Structures remain unchanged
typedef struct SubscriberNode {
    SOCKET socket;
    struct SubscriberNode* next;
} SubscriberNode;

typedef struct HashMapNode {
    char topic[TOPIC_SIZE];
    SubscriberNode* subscribers;
    struct HashMapNode* next;
} HashMapNode;

typedef struct HashMap {
    HashMapNode* buckets[HASH_MAP_SIZE];
} HashMap;

unsigned int hashFunction(const char* topic);

void initializeHashMapWithMutex(HashMap* map);

void insertIntoHashMapWithLock(HashMap* map, const char* topic, SOCKET socket);

SubscriberNode* getSubscribersWithLock(HashMap* map, const char* topic);

void freeHashMapWithMutex(HashMap* map);

// Global mutex for synchronizing hash map operations


#endif // HASHMAP_H

