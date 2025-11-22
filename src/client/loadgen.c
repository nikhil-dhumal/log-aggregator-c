#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "loadgen_worker.h"
#include "loadgen.h"

int run_loadgen(loadgen_config *cfg)
{
    pthread_t *threads = malloc(cfg->threads * sizeof(pthread_t));
    worker_ctx *workers = malloc(cfg->threads * sizeof(worker_ctx));
    worker_metrics *metrics = malloc(cfg->threads * sizeof(worker_metrics));

    for (int i = 0; i < cfg->threads; i++)
    {
        workers[i].thread_id = i;
        workers[i].rate_per_thread = cfg->rate / cfg->threads;
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

    for (int i = 0; i < cfg->threads; i++)
    {
        success_count += metrics[i].success_count;
        failure_count += metrics[i].failure_count;
        total_response_time_us += metrics[i].total_response_time_us;
    }

    long total_count = success_count + failure_count;
    double avg_response_time_us = (total_count > 0) ? (total_response_time_us / total_count) : 0.0;
    double avg_response_time_ms = avg_response_time_us / 1000.0;
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
        fprintf(csv, "workload,threads,rate,duration,success_count,failure_count,avg_response_time_ms,avg_throughput_rps,failure_rate\n");
    }

    fprintf(csv, "%s,%d,%d,%d,%ld,%ld,%.3lf,%.3lf,%.3lf\n", cfg->workload, cfg->threads, cfg->rate, cfg->duration_sec, success_count, failure_count, avg_response_time_ms, avg_throughput_rps, failure_rate);
    fclose(csv);

    free(threads);
    free(workers);
    free(metrics);

    return 0;
}