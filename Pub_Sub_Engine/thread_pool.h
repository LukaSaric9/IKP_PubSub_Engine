#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"

// Thread pool declarations - pure threading concerns
extern HANDLE threadPool[MAX_THREADS];
extern HANDLE clientQueueMutex;
extern SOCKET clientQueue[MAX_THREADS];
extern int queueCount;
extern HANDLE messageThreadPool[MAX_MESSAGE_THREADS];

// Thread pool management functions
void InitializeThreadPool();
void CleanupThreadPool();
void InitializeMessageThreadPool();
void CleanupMessageThreadPool();

// Worker functions
DWORD WINAPI WorkerFunction(LPVOID lpParam);
DWORD WINAPI MessageWorkerFunction(LPVOID lpParam);

// Client queue management
void AddClientToQueue(SOCKET clientSocket);

#endif // THREAD_POOL_H
