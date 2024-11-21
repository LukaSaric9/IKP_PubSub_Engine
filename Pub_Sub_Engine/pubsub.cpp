#include "pubsub.h"
/*
static HashMap* topic_map;
static CircularBuffer* publish_buffer;
static ThreadPool* thread_pool;
static StorageService* storage_service; // Assume defined in storage.h

// Function to process a publish task
static void process_publish_task(void* arg) {
    PublishTask* task = (PublishTask*)arg;
    // Forward message to subscribers
    Subscriber* subs = hashmap_get_subscribers(topic_map, task->topic); //napravi listu
    if (subs) {
        Subscriber* current = subs;
        while (current) {
            // Send message to subscriber
            // Assume send_message is a function that sends the message over the socket
            send_message(current->client_socket, task->topic, task->message);
            current = current->next;
        }
        // Free the subscribers copy
        current = subs;
        while (current) {
            Subscriber* temp = current;
            current = current->next;
            free(temp);
        }
    }

    // Send a copy to the storage service
    storage_store_message(storage_service, task->topic, task->message);

    free(task);
}

void Connect() {
    topic_map = hashmap_init(HASHMAP_SIZE);
    publish_buffer = circular_buffer_init(INITIAL_BUFFER_SIZE);
    thread_pool = thread_pool_init(4); // Number of worker threads can be configured
    storage_service = storage_init(); // Assume storage_init initializes the storage service

    // Start a thread to consume publish tasks
    // Alternatively, integrate with thread pool directly
}

void Subscribe(const char* topic, int client_socket) {
    hashmap_subscribe(topic_map, topic, client_socket);
    LOG_INFO("Client %d subscribed to topic '%s'", client_socket, topic);
}

void Publish(const char* topic, const char* message) {
    // Create a publish task
    PublishTask* task = (PublishTask*)malloc(sizeof(PublishTask));
    CHECK_ALLOC(task);
    strncpy(task->topic, topic, MAX_TOPIC_LENGTH);
    strncpy(task->message, message, MAX_MESSAGE_LENGTH);

    // Add the task to the circular buffer
    PublishTask copy_task = *task; // Make a copy
    circular_buffer_add(publish_buffer, copy_task);
    free(task);

    // Add a processing task to the thread pool
    PublishTask* task_ptr = (PublishTask*)malloc(sizeof(PublishTask));
    CHECK_ALLOC(task_ptr);
    *task_ptr = copy_task;
    thread_pool_add_task(thread_pool, process_publish_task, task_ptr);

    LOG_INFO("Published message to topic '%s'", topic);
}

void PubSub_Cleanup() {
    // Cleanup thread pool
    thread_pool_destroy(thread_pool);

    // Cleanup circular buffer
    circular_buffer_destroy(publish_buffer);

    // Cleanup hashmap
    hashmap_destroy(topic_map);

    // Cleanup storage service
    storage_destroy(storage_service);
}*/
