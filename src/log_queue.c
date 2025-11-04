#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "config.h"
#include "log_queue.h"

static log_entry buffer[QUEUE_CAPACITY];
static int head = 0;
static int tail = 0;
static int count = 0;

static pthread_mutex_t lock;
static pthread_cond_t not_empty;
static pthread_cond_t not_full;

static int queue_running = 1;
static int queue_initialized = 0;

int queue_init(void)
{
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        return -1;
    }
    if (pthread_cond_init(&not_empty, NULL) != 0)
    {
        return -1;
    }
    if (pthread_cond_init(&not_full, NULL) != 0)
    {
        return -1;
    }

    head = tail = count = 0;
    queue_running = 1;
    queue_initialized = 1;

    return 0;
}

int queue_push(log_entry *entry)
{
    pthread_mutex_lock(&lock);

    if (!queue_running)
    {
        pthread_mutex_unlock(&lock);
        return -1;
    }

    while (count == QUEUE_CAPACITY && queue_running)
    {
        pthread_cond_wait(&not_full, &lock);
    }

    if (!queue_running)
    {
        pthread_mutex_unlock(&lock);
        return -1;
    }

    buffer[tail] = *entry;
    tail = (tail + 1) % QUEUE_CAPACITY;
    count++;

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
    return 0;
}

int queue_pop(log_entry *out)
{
    pthread_mutex_lock(&lock);

    if (!queue_running)
    {
        pthread_mutex_unlock(&lock);
        return -1;
    }

    while (count == 0 && queue_running)
    {
        pthread_cond_wait(&not_empty, &lock);
    }

    if (!queue_running && count == 0)
    {
        pthread_mutex_unlock(&lock);
        return -1;
    }

    *out = buffer[head];
    head = (head + 1) % QUEUE_CAPACITY;
    count--;

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);
    return 0;
}

void queue_shutdown(void)
{
    pthread_mutex_lock(&lock);
    queue_running = 0;
    pthread_cond_broadcast(&not_empty);
    pthread_cond_broadcast(&not_full);
    pthread_mutex_unlock(&lock);
}

void queue_destroy(void)
{
    if (!queue_initialized)
    {
        return;
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);
}
