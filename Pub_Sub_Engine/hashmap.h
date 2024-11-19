#ifndef HASHMAP_H
#define HASHMAP_H

#include "common.h"


typedef struct Subscriber {
    int client_socket;
    struct Subscriber* next;
} Subscriber;

typedef struct HashMapEntry {
    char topic[MAX_TOPIC_LENGTH]; // svaki topik ima listu subscribera
    Subscriber* subscribers;
    Mutex mutex; // Protects the subscribers list
    struct HashMapEntry* next; //postoji lista svih topica
} HashMapEntry;

typedef struct HashMap {
    HashMapEntry** buckets; // Array of buckets (linked list of HashMapEntry)
    size_t size;
    Mutex mutex; // Protects the entire hashmap
} HashMap;  

// Initialize the hashmap
HashMap* hashmap_init(size_t size);

// Insert a subscriber to a topic
void hashmap_subscribe(HashMap* map, const char* topic, int client_socket);

// Retrieve subscribers for a topic
Subscriber* hashmap_get_subscribers(HashMap* map, const char* topic);

// Destroy the hashmap and free resources
void hashmap_destroy(HashMap* map);

#endif // HASHMAP_H

