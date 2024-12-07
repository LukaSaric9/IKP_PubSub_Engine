#ifndef COMMON_H
#define COMMON_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>

#define BUFFER_LENGTH 1024
#define DEFAULT_PORT 7000
#define MAX_THREADS 5
#define INITIAL_CAPACITY 1
#define STORAGE_IDENTIFIER "STORAGE"
#define MAX_MESSAGE_THREADS 5

#endif /* COMMON_H */