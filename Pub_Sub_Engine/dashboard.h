#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "common.h"
#include <time.h>

// Statistics structure
typedef struct {
    // Connection statistics
    int active_publishers;
    int active_subscribers;
    int active_storage_services;

    // Message statistics
    int total_messages_processed;
    int messages_last_second;
    int messages_per_priority[3]; // [HIGH, MEDIUM, LOW]
    double average_message_size;
    long long total_message_bytes;

    // Performance statistics
    time_t server_start_time;

    // Topic statistics
    char top_topics[5][64];
    int top_topic_counts[5];

    // Real-time tracking
    time_t last_update_time;
    int messages_in_current_second;
} ServerStatistics;

// Global statistics instance
extern ServerStatistics serverStats;

// Dashboard functions
void initializeStatistics();
void updateConnectionStats(const char* client_type, int connected); // +1 for connect, -1 for disconnect
void updateMessageStats(const char* topic, const char* message, int priority);
void displayDashboard();
void resetStatistics();

// Utility functions
double calculateUptime();
void updateTopTopics(const char* topic);
double calculateMessagesPerMinute(); 

#endif // DASHBOARD_H