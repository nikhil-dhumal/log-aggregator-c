#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>
#include "loadgen_worker.h"

typedef struct
{
    const char *level;
    const char *message;
} log_template;

static log_template logs_table[] = {
    {"INFO", "Log system online"}, {"WARN", "CPU usage high"}, {"ERROR", "Database connection failed"}, {"DEBUG", "Cache updated successfully"}, {"INFO", "User login successful"}, {"WARN", "Memory usage approaching limit"}, {"ERROR", "Failed to write log"}, {"DEBUG", "Worker thread started"}, {"INFO", "Scheduled job executed"}, {"WARN", "Disk space low"}, {"ERROR", "Timeout occurred"}, {"DEBUG", "Cache cleared"}, {"INFO", "New connection established"}, {"WARN", "High latency detected"}, {"ERROR", "Failed to send email"}, {"DEBUG", "Session refreshed"}, {"INFO", "Configuration loaded"}, {"WARN", "Slow query detected"}, {"ERROR", "Data parsing failed"}, {"DEBUG", "Log queue processed"}, {"INFO", "Health check passed"}, {"WARN", "Cache miss rate high"}, {"ERROR", "Permission denied"}, {"DEBUG", "Thread pool expanded"}, {"INFO", "Backup completed"}, {"WARN", "API response slow"}, {"ERROR", "Connection reset"}, {"DEBUG", "Metrics recorded"}, {"INFO", "user logout successful"}, {"WARN", "Retry limit reached"}, {"ERROR", "Disk write failed"}, {"DEBUG", "Worker idle"}, {"INFO", "New user registered"}, {"WARN", "High load on server"}, {"ERROR", "API key invalid"}, {"DEBUG", "Cache hit"}, {"INFO", "Job scheduled"}, {"WARN", "Request timeout"}, {"ERROR", "File not found"}, {"DEBUG", "Session cleaned"}, {"INFO", "Service started"}, {"WARN", "CPU throttling"}, {"ERROR", "Out of memory"}, {"DEBUG", "Health check initiated"}, {"INFO", "Log rotation completed"}, {"WARN", "SSL handshake failed"}, {"ERROR", "Unable to connect to database"}, {"DEBUG", "Queue length monitored"}, {"INFO", "Heartbeat sent"}, {"WARN", "Cache stale data detected"}};
static int logs_table_size = sizeof(logs_table) / sizeof(logs_table[0]);

long long now_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

size_t discard_response(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    (void)ptr;
    (void)userdata;
    return size * nmemb;
}

int send_post_request(CURL *curl, const char *url)
{
    int idx = rand() % logs_table_size;
    char payload[512];
    int n = snprintf(payload, sizeof(payload), "{\"level\":\"%s\",\"message\":\"%s\"}", logs_table[idx].level, logs_table[idx].message);

    if (n < 0 || n >= (int)sizeof(payload))
    {
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);

    CURLcode res = curl_easy_perform(curl);

    return (res == CURLE_OK) ? 0 : -1;
}

int send_get_request(CURL *curl, const char *url)
{
    int idx = rand() % logs_table_size;
    int limit = 1 + rand() % 100;
    int page = 1 + rand() % 10;
    char get_url[512];
    int n = snprintf(get_url, sizeof(get_url), "%s?limit=%d&level=%s&page=%d", url, limit, logs_table[idx].level, page);

    if (n < 0 || n >= (int)sizeof(get_url))
    {
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, get_url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);

    CURLcode res = curl_easy_perform(curl);

    return (res == CURLE_OK) ? 0 : -1;
}

void *loadgen_worker(void *args)
{
    worker_ctx *ctx = (worker_ctx *)args;

    unsigned int thread_seed = (unsigned int)time(NULL) ^ (unsigned int)(ctx->thread_id * 1103515245u);
    srand(thread_seed);

    int success_count = 0;
    int failure_count = 0;
    long long total_response_time_us = 0;
    long long min_response_time_us = LLONG_MAX;
    long long max_response_time_us = 0;
    long long start_us = now_us();
    long long end_us = start_us + ctx->duration_sec * 1000000LL;

    CURL *curl = curl_easy_init();

    if (!curl)
    {
        worker_metrics *m = malloc(sizeof(worker_metrics));
        if (!m)
        {
            return NULL;
        }
        m->success_count = 0;
        m->failure_count = 0;
        m->total_response_time_us = 0;
        m->min_response_time_us = 0;
        m->max_response_time_us = 0;
        return m;
    }

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_response);

    while (now_us() < end_us)
    {
        long long req_start = now_us();
        int res = -1;

        double r = ((double)rand() / RAND_MAX);

        if (strcmp(ctx->workload, "write-heavy") == 0)
        {
            send_post_request(curl, ctx->url);
        }
        else if (strcmp(ctx->workload, "read-heavy") == 0)
        {
            res = (r < 0.8) ? send_get_request(curl, ctx->url) : send_post_request(curl, ctx->url);
        }
        else
        {
            res = (r < 0.5) ? send_post_request(curl, ctx->url) : send_get_request(curl, ctx->url);
        }

        if (res == 0)
        {
            success_count++;
        }
        else
        {
            failure_count++;
        }

        long long req_end = now_us();
        long long elapsed = req_end - req_start;

        if (elapsed < 0)
            elapsed = 0;

        total_response_time_us += elapsed;
        if (elapsed > max_response_time_us)
        {
            max_response_time_us = elapsed;
        }
        if (elapsed < min_response_time_us)
        {
            min_response_time_us = elapsed;
        }
    }

    if (min_response_time_us == LLONG_MAX)
    {
        min_response_time_us = 0;
    }

    worker_metrics *metrics = malloc(sizeof(worker_metrics));

    if (!metrics)
    {
        return NULL;
    }

    metrics->success_count = success_count;
    metrics->failure_count = failure_count;
    metrics->total_response_time_us = total_response_time_us;
    metrics->max_response_time_us = max_response_time_us;
    metrics->min_response_time_us = min_response_time_us;

    return metrics;
}