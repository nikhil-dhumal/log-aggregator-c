#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "cache.h"
#include "config.h"

static log_entry *cache = NULL;
static int capacity;
static int count;
static int head;
static int tail;

static pthread_rwlock_t lock;

int cache_init(int size)
{
    capacity = size;
    cache = malloc(size * sizeof(log_entry));
    if (!cache)
    {
        fprintf(stderr, "[cache] ERROR: malloc failed\n");
        return -1;
    }

    count = 0;
    head = 0;
    tail = 0;

    if (pthread_rwlock_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "[cache] ERROR: mutex init failed\n");
        free(cache);
        cache = NULL;
        return -1;
    }

    return 0;
}

void cache_destroy(void)
{
    if (cache)
    {
        free(cache);
    }
    cache = NULL;
    pthread_rwlock_destroy(&lock);
}

void cache_insert(log_entry *entry)
{
    pthread_rwlock_wrlock(&lock);
    cache[tail] = *entry;
    tail = (tail + 1) % capacity;
    if (count < capacity)
    {
        count++;
    }
    else
    {
        head = (head + 1) % capacity;
    }
    pthread_rwlock_unlock(&lock);
}

int cache_get_last(int limit, const char *level, log_entry *buffer)
{
    pthread_rwlock_rdlock(&lock);
    if (count == 0)
    {
        pthread_rwlock_unlock(&lock);
        return 0;
    }
    int matched = 0;
    int scanned = 0;
    int index = (tail - 1 + capacity) % capacity;
    if (limit > count)
    {
        limit = count;
    }
    while (matched < limit && scanned < count)
    {
        if (level == NULL || strcmp(level, cache[index].level) == 0)
        {
            buffer[matched] = cache[index];
            matched++;
        }
        index = (index - 1 + capacity) % capacity;
        scanned++;

    }
    pthread_rwlock_unlock(&lock);
    return matched;
}
