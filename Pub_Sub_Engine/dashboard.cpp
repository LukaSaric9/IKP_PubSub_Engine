#define _CRT_SECURE_NO_WARNINGS
#include "dashboard.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Global statistics instance
ServerStatistics serverStats;

void initializeStatistics() {
    memset(&serverStats, 0, sizeof(ServerStatistics));
    serverStats.server_start_time = time(NULL);
    serverStats.last_update_time = serverStats.server_start_time;

    // Initialize top topics 
    for (int i = 0; i < 5; i++) {
        strcpy_s(serverStats.top_topics[i], sizeof(serverStats.top_topics[i]), "");
        serverStats.top_topic_counts[i] = 0;
    }

    printf("*** SERVER STATISTICS INITIALIZED ***\n");
}

void updateConnectionStats(const char* client_type, int connected) {
    if (connected > 0) {
        if (strstr(client_type, "PUBLISHER") != NULL) {
            serverStats.active_publishers++;
        }
        else if (strstr(client_type, "SUBSCRIBER") != NULL) {
            serverStats.active_subscribers++;
        }
        else if (strstr(client_type, "STORAGE") != NULL) {
            serverStats.active_storage_services++;
        }
    }
    else {
        if (strstr(client_type, "PUBLISHER") != NULL && serverStats.active_publishers > 0) {
            serverStats.active_publishers--;
        }
        else if (strstr(client_type, "SUBSCRIBER") != NULL && serverStats.active_subscribers > 0) {
            serverStats.active_subscribers--;
        }
        else if (strstr(client_type, "STORAGE") != NULL && serverStats.active_storage_services > 0) {
            serverStats.active_storage_services--;
        }
    }
}


void updateMessageStats(const char* topic, const char* message, int priority) {
    serverStats.total_messages_processed++;

    // Update priority statistics
    if (priority >= 0 && priority <= 2) {
        serverStats.messages_per_priority[priority]++;
    }

    // Calculate message size
    size_t messageSize = strlen(topic) + strlen(message) + 2; // +2 for ": "
    serverStats.total_message_bytes += messageSize;
    serverStats.average_message_size = (double)serverStats.total_message_bytes / serverStats.total_messages_processed;

    // Update top topics
    updateTopTopics(topic);
}

// Calculate messages per minute based on total uptime
double calculateMessagesPerMinute() {
    double uptimeInMinutes = calculateUptime() / 60.0;
    if (uptimeInMinutes > 0) {
        return (double)serverStats.total_messages_processed / uptimeInMinutes;
    }
    return 0.0;
}

void displayDashboard() {
    system("cls"); // Clear screen (Windows)

    printf("+========================================================+\n");
    printf("|                 PUBSUB SERVER DASHBOARD               |\n");
    printf("+========================================================+\n");

    // Server uptime
    double uptime = calculateUptime();
    int hours = (int)(uptime / 3600);
    int minutes = (int)((uptime - hours * 3600) / 60);
    int seconds = (int)(uptime - hours * 3600 - minutes * 60);

    printf("| Server Uptime: %02d:%02d:%02d                           |\n", hours, minutes, seconds);
    printf("+========================================================+\n");

    // Connection Statistics
    printf("| CONNECTION STATISTICS                                  |\n");
    printf("+--------------------------------------------------------+\n");
    printf("| Active Publishers:     %3d                            |\n", serverStats.active_publishers);
    printf("| Active Subscribers:    %3d                            |\n", serverStats.active_subscribers);
    printf("| Active Storage Svcs:   %3d                            |\n", serverStats.active_storage_services);
    printf("| Total Connections:     %3d                            |\n",
        serverStats.active_publishers + serverStats.active_subscribers + serverStats.active_storage_services);
    printf("+========================================================+\n");

    // Message Statistics
    printf("| MESSAGE STATISTICS                                     |\n");
    printf("+--------------------------------------------------------+\n");
    printf("| Messages/Minute:       %5.1f                          |\n", calculateMessagesPerMinute());
    printf("| Total Messages:        %3d                            |\n", serverStats.total_messages_processed);
    printf("| Avg Message Size:      %.1f bytes                     |\n", serverStats.average_message_size);
    printf("+========================================================+\n");

    // Priority Distribution
    printf("| PRIORITY DISTRIBUTION                                  |\n");
    printf("+--------------------------------------------------------+\n");
    printf("| [H] HIGH Priority:     %3d (%5.1f%%)                   |\n",
        serverStats.messages_per_priority[0],
        serverStats.total_messages_processed > 0 ?
        (double)serverStats.messages_per_priority[0] / serverStats.total_messages_processed * 100 : 0);
    printf("| [M] MEDIUM Priority:   %3d (%5.1f%%)                   |\n",
        serverStats.messages_per_priority[1],
        serverStats.total_messages_processed > 0 ?
        (double)serverStats.messages_per_priority[1] / serverStats.total_messages_processed * 100 : 0);
    printf("| [L] LOW Priority:      %3d (%5.1f%%)                   |\n",
        serverStats.messages_per_priority[2],
        serverStats.total_messages_processed > 0 ?
        (double)serverStats.messages_per_priority[2] / serverStats.total_messages_processed * 100 : 0);
    printf("+========================================================+\n");

    // Top Topics
    printf("| TOP 5 MOST ACTIVE TOPICS                              |\n");
    printf("+--------------------------------------------------------+\n");
    for (int i = 0; i < 5; i++) {
        if (strlen(serverStats.top_topics[i]) > 0) {
            printf("| %d. %-20s %3d messages                  |\n",
                i + 1, serverStats.top_topics[i], serverStats.top_topic_counts[i]);
        }
        else {
            printf("| %d. %-20s %3s                          |\n", i + 1, "-", "");
        }
    }
    printf("+========================================================+\n");

    printf("| Press 'd' to toggle dashboard, 'r' to reset stats    |\n");
    printf("+========================================================+\n");
}

double calculateUptime() {
    return difftime(time(NULL), serverStats.server_start_time);
}

void updateTopTopics(const char* topic) {
    int topicIndex = -1;
    
    // Find if topic already exists
    for (int i = 0; i < 5; i++) {
        if (strcmp(serverStats.top_topics[i], topic) == 0) {
            topicIndex = i;
            serverStats.top_topic_counts[i]++;
            break;
        }
    }
    
    if (topicIndex == -1) {
        // Topic not found, need to add it
        // Find first empty slot or replace the one with lowest count
        int insertIndex = -1;
        int minCount = INT_MAX;
        int minIndex = 0;
        
        for (int i = 0; i < 5; i++) {
            if (strlen(serverStats.top_topics[i]) == 0) {
                insertIndex = i;
                break;
            }
            if (serverStats.top_topic_counts[i] < minCount) {
                minCount = serverStats.top_topic_counts[i];
                minIndex = i;
            }
        }
        
        if (insertIndex == -1) {
            // No empty slots, replace the one with minimum count
            insertIndex = minIndex;
        }
        
        strcpy_s(serverStats.top_topics[insertIndex], sizeof(serverStats.top_topics[insertIndex]), topic);
        serverStats.top_topic_counts[insertIndex] = 1;
        topicIndex = insertIndex;
    }
    
    // Bubble sort to maintain order (highest count first)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4 - i; j++) {
            if (serverStats.top_topic_counts[j] < serverStats.top_topic_counts[j + 1]) {
                // Swap topics
                char tempTopic[64];
                strcpy_s(tempTopic, sizeof(tempTopic), serverStats.top_topics[j]);
                strcpy_s(serverStats.top_topics[j], sizeof(serverStats.top_topics[j]), serverStats.top_topics[j + 1]);
                strcpy_s(serverStats.top_topics[j + 1], sizeof(serverStats.top_topics[j + 1]), tempTopic);
                
                // Swap counts
                int tempCount = serverStats.top_topic_counts[j];
                serverStats.top_topic_counts[j] = serverStats.top_topic_counts[j + 1];
                serverStats.top_topic_counts[j + 1] = tempCount;
            }
        }
    }
}

void resetStatistics() {
    int active_pubs = serverStats.active_publishers;
    int active_subs = serverStats.active_subscribers;
    int active_storage = serverStats.active_storage_services;
    time_t start_time = serverStats.server_start_time;

    memset(&serverStats, 0, sizeof(ServerStatistics));

    // Restore connection counts and start time
    serverStats.active_publishers = active_pubs;
    serverStats.active_subscribers = active_subs;
    serverStats.active_storage_services = active_storage;
    serverStats.server_start_time = start_time;
    serverStats.last_update_time = time(NULL);

    printf("*** STATISTICS RESET ***\n");
}