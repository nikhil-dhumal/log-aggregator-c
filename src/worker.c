#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include "worker.h"
#include "queue.h"
#include "log_entry.h"
#include "storage.h"
#include "cache.h"

static pthread_t worker_thread;

static atomic_int worker_running = 1;

static void *worker_loop(void *arg)
{
    (void)arg;

    while (worker_running)
    {
        log_entry entry;
        if (queue_pop(&entry) != 0)
        {
            break;
        }
        storage_write(&entry);
        cache_insert(&entry);
    }

    return NULL;
}

void worker_start(void)
{
    worker_running = 1;
    storage_init_writer();
    pthread_create(&worker_thread, NULL, worker_loop, NULL);
}

void worker_stop(void)
{
    worker_running = 0;
    pthread_join(worker_thread, NULL);
}