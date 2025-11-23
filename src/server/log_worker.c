#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>
#include <config.h>
#include "cache.h"
#include "db.h"
#include "log_entry.h"
#include "log_queue.h"
#include "log_worker.h"

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
        cache_insert(&entry);
        db_write(&entry);
    }

    return NULL;
}

void worker_start(void)
{
    worker_running = 1;
    if (db_init_writer() != 0)
    {
        fprintf(stderr, "[worker] ERROR: DB writer init failed\n");
        return;
    }
    if (pthread_create(&worker_thread, NULL, worker_loop, NULL) != 0)
    {
        fprintf(stderr, "[worker] ERROR: thread create failed\n");
    }
}

void worker_stop(void)
{
    worker_running = 0;
    pthread_join(worker_thread, NULL);
}
