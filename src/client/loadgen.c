#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "loadgen_worker.h"
#include "loadgen.h"

int run_loadgen(loadgen_config *cfg)
{
    pthread_t *threads = malloc(cfg->threads * sizeof(pthread_t));

    if (!threads)
    {
        fprintf(stderr, "[loadgen] ERROR: malloc threads failed\n");
        return -1;
    }

    worker_ctx *workers = malloc(cfg->threads * sizeof(worker_ctx));

    if (!workers)
    {
        fprintf(stderr, "[loadgen] ERROR: malloc workers failed\n");
        free(threads);
        return -1;
    }

    worker_metrics *metrics = malloc(cfg->threads * sizeof(worker_metrics));

    if (!metrics)
    {
        fprintf(stderr, "[loadgen] ERROR: malloc metrics failed\n");
        free(threads);
        free(workers);
        return -1;
    }

    for (int i = 0; i < cfg->threads; i++)
    {
        workers[i].thread_id = i;
        workers[i].duration_sec = cfg->duration_sec;
        workers[i].url = cfg->url;
        workers[i].workload = cfg->workload;
    }

    for (int i = 0; i < cfg->threads; i++)
    {
        pthread_create(&threads[i], NULL, loadgen_worker, (void *)&workers[i]);
    }

    worker_metrics *metric;

    for (int i = 0; i < cfg->threads; i++)
    {
        pthread_join(threads[i], (void **)&metric);
        metrics[i] = *metric;
        free(metric);
    }

    long success_count = 0;
    long failure_count = 0;
    double total_response_time_us = 0.0;
    long long global_min_us = LLONG_MAX;
    long long global_max_us = 0;

    for (int i = 0; i < cfg->threads; i++)
    {
        success_count += metrics[i].success_count;
        failure_count += metrics[i].failure_count;
        total_response_time_us += metrics[i].total_response_time_us;
        if (metrics[i].min_response_time_us >= 0 && metrics[i].min_response_time_us < global_min_us)
        {
            global_min_us = metrics[i].min_response_time_us;
        }
        if (metrics[i].max_response_time_us > global_max_us)
        {
            global_max_us = metrics[i].max_response_time_us;
        }
    }

    long total_count = success_count + failure_count;
    double avg_response_time_us = (total_count > 0) ? (total_response_time_us / total_count) : 0.0;
    double avg_response_time_ms = avg_response_time_us / 1000.0;
    double min_response_time_ms = (global_min_us == LLONG_MAX) ? 0.0 : ((double)global_min_us / 1000.0);
    double max_response_time_ms = ((double)global_max_us / 1000.0);
    double avg_throughput_rps = (double)success_count / cfg->duration_sec;
    double failure_rate = (double)failure_count / (success_count + failure_count) * 100.0;

    FILE *csv = fopen(cfg->csv_path, "r");
    int write_header = 0;

    if (!csv)
    {
        write_header = 1;
    }
    else
    {
        fclose(csv);
    }

    csv = fopen(cfg->csv_path, "a");

    if (!csv)
    {
        fprintf(stderr, "[loadgen] ERROR: file open failed");
        return -1;
    }

    if (write_header)
    {
        fprintf(csv, "workload,threads,duration,success_count,failure_count,avg_response_time_ms,max_response_time_ms,min_response_time_ms,avg_throughput_rps,failure_rate\n");
    }

    fprintf(csv, "%s,%d,%d,%ld,%ld,%.3f,%.3f,%.3f,%.3f,%.3f\n", cfg->workload, cfg->threads, cfg->duration_sec, success_count, failure_count, avg_response_time_ms, max_response_time_ms, min_response_time_ms, avg_throughput_rps, failure_rate);
    fclose(csv);

    free(threads);
    free(workers);
    free(metrics);

    return 0;
}