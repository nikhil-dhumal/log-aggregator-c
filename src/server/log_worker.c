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

long long now_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000);
}

static void *worker_loop(void *arg)
{
    (void)arg;

    int count = 0;
    log_entry entires[WRITE_BATCH_SIZE];
    long long batch_start = now_ms();

    while (worker_running)
    {
        log_entry entry;
        if (queue_pop(&entry) != 0)
        {
            if (count > 0) {
                db_write(entires, count);
            }
            break;
        }
        long long now = now_ms();
        entires[count++] = entry;
        cache_insert(&entry);
        if (count == WRITE_BATCH_SIZE || now - batch_start >= 100)
        {
            db_write(entires, count);
            count = 0;
            batch_start = now_ms();
        }
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
