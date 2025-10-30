#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include "queue.h"

#define QUEUE_CAPACITY 1024

static log_entry buffer[QUEUE_CAPACITY];
static int head = 0;
static int tail = 0;
static int count = 0;

static pthread_mutex_t lock;
static pthread_cond_t not_empty;
static pthread_cond_t not_full;

void queue_init(void)
{
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);
}

int queue_push(log_entry *entry)
{
    pthread_mutex_lock(&lock);

    while (count == QUEUE_CAPACITY)
    {
        pthread_cond_wait(&not_full, &lock);
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

    while (count == 0)
    {
        pthread_cond_wait(&not_empty, &lock);
    }

    *out = buffer[head];
    head = (head + 1) % QUEUE_CAPACITY;
    count--;

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);
    return 0;
}

void queue_destroy(void)
{
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);
}