#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>
#include "loadgen_worker.h"

long long now_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

size_t discard_response(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    (void)ptr;
    (void)userdata;
    return size * nmemb;
}

int send_post_request(const char *url, const char *payload)
{
    CURL *curl = curl_easy_init();
    
    if (!curl)
    {
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK) ? 0 : -1;
}

int send_get_request(const char *url)
{
    CURL *curl = curl_easy_init();

    if (!curl)
    {
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return (res == CURLE_OK) ? 0 : -1;
}

void *loadgen_worker(void *args)
{
    worker_ctx *ctx = (worker_ctx *) args;

    int success_count = 0;
    int failure_count = 0;
    long long total_response_time_us = 0;
    long long start_us = now_us();
    long long end_us = start_us + ctx->duration_sec * 1000000LL;
    long long interval_us = 1000000LL / ctx->rate_per_thread;

    while (now_us() < end_us)
    {
        long long req_start = now_us();

        int res = send_post_request(ctx->url, "{\"level\":\"INFO\",\"message\":\"load tes\"}");
        // int res = send_get_request(ctx->url);

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

        total_response_time_us += elapsed;

        if (elapsed < interval_us)
        {
            usleep(interval_us - elapsed);
        }
    }

    worker_metrics *metrics = malloc(sizeof(worker_metrics));

    if (!metrics)
    {
        return NULL;
    }

    metrics->success_count = success_count;
    metrics->failure_count = failure_count;
    metrics->total_response_time_us = total_response_time_us;

    return metrics;
}