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

static pthread_mutex_t lock;

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

    if (pthread_mutex_init(&lock, NULL) != 0)
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
    pthread_mutex_destroy(&lock);
}

void cache_insert(log_entry *entry)
{
    pthread_mutex_lock(&lock);
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
    pthread_mutex_unlock(&lock);
}

int cache_get_last(int n, log_entry *buffer)
{
    pthread_mutex_lock(&lock);
    if (count == 0)
    {
        pthread_mutex_unlock(&lock);
        return 0;
    }
    if (n > count)
    {
        n = count;
    }
    int start = (tail - n + capacity) % capacity;
    for (int i = 0; i < n; i++)
    {
        int index = (start + i) % capacity;
        buffer[i] = cache[index];
    }
    pthread_mutex_unlock(&lock);
    return n;
}
