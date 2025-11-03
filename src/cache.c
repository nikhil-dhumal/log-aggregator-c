#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "cache.h"

static log_entry *cache = NULL;
static int capacity;
static int count;
static int head;
static int tail;

static pthread_mutex_t lock;

void cache_init(int size)
{
    capacity = size;
    cache = (log_entry *)malloc(size * sizeof(log_entry));
    count = 0;
    head = 0;
    tail = 0;
    pthread_mutex_init(&lock, NULL);
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
    else {
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
    for (int i = 0; i < n; i++) {
        int index = (start + i) % capacity;
        buffer[i] = cache[index];
    }
    pthread_mutex_unlock(&lock);
    return n;
}