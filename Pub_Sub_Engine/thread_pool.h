#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"
#include "hashmap.h"

// Thread pool and client queue declarations
extern HANDLE threadPool[MAX_THREADS];
extern HANDLE clientQueueMutex;
extern SOCKET clientQueue[MAX_THREADS];
extern int queueCount;
extern HashMap topicSubscribers;

void InitializeThreadPool();
void CleanupThreadPool();
DWORD WINAPI WorkerFunction(LPVOID lpParam);
void InitializeGlobalData();
void CleanupGlobalData();

#endif // THREAD_POOL_H
