#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"

// Thread pool and client queue declarations
extern HANDLE threadPool[MAX_THREADS];
extern HANDLE clientQueueMutex;
extern SOCKET clientQueue[MAX_THREADS];
extern int queueCount;

void InitializeThreadPool();
void CleanupThreadPool();
DWORD WINAPI WorkerFunction(LPVOID lpParam);

#endif // THREAD_POOL_H
