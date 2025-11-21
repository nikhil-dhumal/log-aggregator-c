#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loadgen.h"

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        fprintf(stderr, "[loadgen] USAGE: %s <url> <rate> <threads> <duraction_sec> <results_csv>\n", argv[0]);
        return 1;
    }

    loadgen_config cfg;

    cfg.url = argv[1];
    cfg.rate = atoi(argv[2]);
    cfg.threads = atoi(argv[3]);
    cfg.duration_sec = atoi(argv[4]);
    cfg.csv_path = argv[5];

    if (cfg.rate <= 0 || cfg.threads <= 0 || cfg.duration_sec <= 0)
    {
        fprintf(stderr, "[loadgen] ERROR: Invalid numeric arguments\n");
        return 1;
    }

    printf("[loadgen] Starting load test: %d requests/sec, %d threads, %d seconds\n", cfg.rate, cfg.threads, cfg.duration_sec);

    int res = run_loadgen(&cfg);

    if (res != 0)
    {
        fprintf(stderr, "[loadgen] ERROR: load generator failed\n");
        return 1;
    }

    printf("[loadgen] Load test completed. Results saved to %s\n", cfg.csv_path);

    return 0;
}