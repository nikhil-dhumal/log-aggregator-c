#ifndef LOADGEN_WORKER_H
#define LOADGEN_WORKER_H

typedef struct
{
    int success_count;
    int failure_count;
    long long total_response_time_us;
    long long min_response_time_us;
    long long max_response_time_us;
} worker_metrics;

typedef struct
{
    int thread_id;
    int duration_sec;
    const char *url;
    const char *workload;
} worker_ctx;

void *loadgen_worker(void *arg);

#endif