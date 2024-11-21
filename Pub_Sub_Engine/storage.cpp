#include "storage.h"
/*
StorageService* storage_init() {
    StorageService* service = (StorageService*)malloc(sizeof(StorageService));
    CHECK_ALLOC(service);
    service->capacity = 1024; // Initial capacity
    service->count = 0;
    service->messages = (StoredMessage**)calloc(service->capacity, sizeof(StoredMessage*));
    CHECK_ALLOC(service->messages);
    MUTEX_INIT(service->mutex);
    return service;
}

void storage_store_message(StorageService* service, const char* topic, const char* message) {
    MUTEX_LOCK(service->mutex);
    if (service->count == service->capacity) {
        // Expand capacity
        size_t new_capacity = service->capacity * 2;
        StoredMessage** new_messages = (StoredMessage**)realloc(service->messages, new_capacity * sizeof(StoredMessage*));
        CHECK_ALLOC(new_messages);
        memset(new_messages + service->capacity, 0, service->capacity * sizeof(StoredMessage*));
        service->messages = new_messages;
        service->capacity = new_capacity;
    }

    // Create a new stored message
    StoredMessage* stored = (StoredMessage*)malloc(sizeof(StoredMessage));
    CHECK_ALLOC(stored);
    strncpy(stored->topic, topic, MAX_TOPIC_LENGTH);
    strncpy(stored->message, message, MAX_MESSAGE_LENGTH);
    strncpy(stored->timestamp, get_timestamp(), sizeof(stored->timestamp));
    stored->next = service->messages[service->count];
    service->messages[service->count] = stored;
    service->count++;
    MUTEX_UNLOCK(service->mutex);
}

StoredMessage* storage_get_messages(StorageService* service, const char* topic) {
    MUTEX_LOCK(service->mutex);
    StoredMessage* head = NULL;
    for (size_t i = 0; i < service->count; ++i) {
        StoredMessage* current = service->messages[i];
        while (current) {
            if (strcmp(current->topic, topic) == 0) {
                // Add to the result list
                StoredMessage* copy = (StoredMessage*)malloc(sizeof(StoredMessage));
                CHECK_ALLOC(copy);
                *copy = *current;
                copy->next = head;
                head = copy;
            }
            current = current->next;
        }
    }
    MUTEX_UNLOCK(service->mutex);
    return head;
}

void storage_destroy(StorageService* service) {
    for (size_t i = 0; i < service->count; ++i) {
        StoredMessage* current = service->messages[i];
        while (current) {
            StoredMessage* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(service->messages);
    MUTEX_DESTROY(service->mutex);
    free(service);
}
*/