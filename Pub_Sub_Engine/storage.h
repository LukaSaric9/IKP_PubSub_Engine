#ifndef STORAGE_H
#define STORAGE_H

#include "common.h"
/*
typedef struct StoredMessage {
    char topic[MAX_TOPIC_LENGTH];
    char message[MAX_MESSAGE_LENGTH];
    char timestamp[20];
    struct StoredMessage* next;
} StoredMessage;

typedef struct StorageService {
    StoredMessage** messages;
    size_t capacity;
    size_t count;
    Mutex mutex;
} StorageService;

// Initialize the storage service
StorageService* storage_init();

// Store a message
void storage_store_message(StorageService* service, const char* topic, const char* message);

// Retrieve messages by topic
StoredMessage* storage_get_messages(StorageService* service, const char* topic);

// Destroy the storage service
void storage_destroy(StorageService* service);
*/
#endif // STORAGE_H

