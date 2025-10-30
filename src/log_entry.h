#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <time.h>

typedef struct
{
    char level[16];
    char message[256];
    time_t timestamp;
} log_entry;

#endif