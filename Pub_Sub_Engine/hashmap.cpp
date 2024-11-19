#include "hashmap.h"

HashMap* hashmap_init(size_t size) {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    CHECK_ALLOC(map);
    map->size = size;
    map->buckets = (HashMapEntry**)calloc(size, sizeof(HashMapEntry*));
    CHECK_ALLOC(map->buckets);
    MUTEX_INIT(map->mutex);
    return map;
}

static size_t hash_function(const char* key, size_t size) {
    size_t hash = 0;
    while (*key) {
        hash = (hash * 31) + *key++;
    }
    return hash % size;
}

void hashmap_subscribe(HashMap* map, const char* topic, int client_socket) {
    size_t index = hash_function(topic, map->size);
    MUTEX_LOCK(map->mutex);
    HashMapEntry* entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            break;
        }
        entry = entry->next;
    }
    if (!entry) {
        // Create a new entry
        entry = (HashMapEntry*)malloc(sizeof(HashMapEntry));
        CHECK_ALLOC(entry);
        strncpy(entry->topic, topic, MAX_TOPIC_LENGTH);
        entry->subscribers = NULL;
        MUTEX_INIT(entry->mutex);
        entry->next = map->buckets[index];
        map->buckets[index] = entry;
    }
    MUTEX_LOCK(entry->mutex);
    MUTEX_UNLOCK(map->mutex); // Release map mutex early

    // Add the subscriber
    Subscriber* sub = (Subscriber*)malloc(sizeof(Subscriber));
    CHECK_ALLOC(sub);
    sub->client_socket = client_socket;
    sub->next = entry->subscribers;
    entry->subscribers = sub;

    MUTEX_UNLOCK(entry->mutex);
}

Subscriber* hashmap_get_subscribers(HashMap* map, const char* topic) {
    size_t index = hash_function(topic, map->size);
    MUTEX_LOCK(map->mutex);
    HashMapEntry* entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->topic, topic) == 0) {
            break;
        }
        entry = entry->next;
    }
    if (!entry) {
        MUTEX_UNLOCK(map->mutex);
        return NULL;
    }
    MUTEX_LOCK(entry->mutex);
    MUTEX_UNLOCK(map->mutex); // Release map mutex early

    // Create a copy of subscribers list to return
    Subscriber* copy = NULL;
    Subscriber* current = entry->subscribers;
    while (current) {
        Subscriber* new_sub = (Subscriber*)malloc(sizeof(Subscriber));
        CHECK_ALLOC(new_sub);
        new_sub->client_socket = current->client_socket;
        new_sub->next = copy;
        copy = new_sub;
        current = current->next;
    }

    MUTEX_UNLOCK(entry->mutex);
    return copy;
}

void hashmap_destroy(HashMap* map) {
    for (size_t i = 0; i < map->size; ++i) {
        HashMapEntry* entry = map->buckets[i];
        while (entry) {
            HashMapEntry* temp = entry;
            entry = entry->next;

            MUTEX_LOCK(temp->mutex);
            Subscriber* sub = temp->subscribers;
            while (sub) {
                Subscriber* sub_temp = sub;
                sub = sub->next;
                free(sub_temp);
            }
            MUTEX_UNLOCK(temp->mutex);
            MUTEX_DESTROY(temp->mutex);
            free(temp);
        }
    }
    free(map->buckets);
    MUTEX_DESTROY(map->mutex);
    free(map);
}
