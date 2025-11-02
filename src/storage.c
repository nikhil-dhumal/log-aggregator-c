#include <stdio.h>
#include <pthread.h>
#include "storage.h"

pthread_mutex_t lock;
static FILE *storage_file;

int storage_init(const char *filename)
{
    storage_file = fopen(filename, "a");
    if (storage_file == NULL)
    {
        fprintf(stderr, "ERROR: Failed to open file: %s\n", filename);
        return -1;
    }
    pthread_mutex_init(&lock, NULL);
    return 0;
}

void storage_write(log_entry *entry)
{
    pthread_mutex_lock(&lock);
    fprintf(storage_file, "%ld %s %s\n", entry->timestamp, entry->level, entry->message);
    fflush(storage_file);
    pthread_mutex_unlock(&lock);
}

void storage_close(void)
{
    fclose(storage_file);
    pthread_mutex_destroy(&lock);
}